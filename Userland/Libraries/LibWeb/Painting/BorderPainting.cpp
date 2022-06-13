/*
 * Copyright (c) 2020, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2021, Sam Atkins <atkinssj@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Painter.h>
#include <LibWeb/Painting/BorderPainting.h>
#include <LibWeb/Painting/PaintContext.h>
#include <LibGfx/AntiAliasingPainter.h>

namespace Web::Painting {

BorderRadiiData normalized_border_radii_data(Layout::Node const& node, Gfx::FloatRect const& rect, CSS::BorderRadiusData top_left_radius, CSS::BorderRadiusData top_right_radius, CSS::BorderRadiusData bottom_right_radius, CSS::BorderRadiusData bottom_left_radius, RelativeToWidthOnly relative_to_width_only)
{
    BorderRadiusData bottom_left_radius_px {};
    BorderRadiusData bottom_right_radius_px {};
    BorderRadiusData top_left_radius_px {};
    BorderRadiusData top_right_radius_px {};

    auto width_length = CSS::Length::make_px(rect.width());
    bottom_left_radius_px.horizontal_radius = bottom_left_radius.horizontal_radius.resolved(node, width_length).to_px(node);
    bottom_right_radius_px.horizontal_radius = bottom_right_radius.horizontal_radius.resolved(node, width_length).to_px(node);
    top_left_radius_px.horizontal_radius = top_left_radius.horizontal_radius.resolved(node, width_length).to_px(node);
    top_right_radius_px.horizontal_radius = top_right_radius.horizontal_radius.resolved(node, width_length).to_px(node);

    // FIXME: Remove `relative_to_width_only = Yes' flag, this only exists to
    // avoid overlapping curves for (outline) borders, which do not yet
    // support elliptical corners.
    switch (relative_to_width_only) {
    case RelativeToWidthOnly::No: {
        // Normal correct rendering:
        auto height_length = CSS::Length::make_px(rect.height());
        bottom_left_radius_px.vertical_radius = bottom_left_radius.vertical_radius.resolved(node, height_length).to_px(node);
        bottom_right_radius_px.vertical_radius = bottom_right_radius.vertical_radius.resolved(node, height_length).to_px(node);
        top_left_radius_px.vertical_radius = top_left_radius.vertical_radius.resolved(node, height_length).to_px(node);
        top_right_radius_px.vertical_radius = top_right_radius.vertical_radius.resolved(node, height_length).to_px(node);
        break;
    }
    case RelativeToWidthOnly::Yes:
        bottom_left_radius_px.vertical_radius = bottom_left_radius_px.horizontal_radius;
        bottom_right_radius_px.vertical_radius = bottom_right_radius_px.horizontal_radius;
        top_left_radius_px.vertical_radius = top_left_radius_px.horizontal_radius;
        top_right_radius_px.vertical_radius = top_right_radius_px.horizontal_radius;
        break;
    default:
        VERIFY_NOT_REACHED();
    }

    // Scale overlapping curves according to https://www.w3.org/TR/css-backgrounds-3/#corner-overlap
    auto f = 1.0f;
    auto width_reciprocal = 1.0f / rect.width();
    auto height_reciprocal = 1.0f / rect.height();
    f = max(f, width_reciprocal * (top_left_radius_px.horizontal_radius + top_right_radius_px.horizontal_radius));
    f = max(f, height_reciprocal * (top_right_radius_px.vertical_radius + bottom_right_radius_px.vertical_radius));
    f = max(f, width_reciprocal * (bottom_left_radius_px.horizontal_radius + bottom_right_radius_px.horizontal_radius));
    f = max(f, height_reciprocal * (top_left_radius_px.vertical_radius + bottom_left_radius_px.vertical_radius));

    f = 1.0f / f;

    top_left_radius_px.horizontal_radius *= f;
    top_left_radius_px.vertical_radius *= f;
    top_right_radius_px.horizontal_radius *= f;
    top_right_radius_px.vertical_radius *= f;
    bottom_right_radius_px.horizontal_radius *= f;
    bottom_right_radius_px.vertical_radius *= f;
    bottom_left_radius_px.horizontal_radius *= f;
    bottom_left_radius_px.vertical_radius *= f;

    return BorderRadiiData { top_left_radius_px, top_right_radius_px, bottom_right_radius_px, bottom_left_radius_px };
}

void paint_border(PaintContext& context, BorderEdge edge, Gfx::IntRect const& rect, BorderRadiiData const& border_radii_data, BordersData const& borders_data)
{
    auto const& border_data = [&] {
        switch (edge) {
        case BorderEdge::Top:
            return borders_data.top;
        case BorderEdge::Right:
            return borders_data.right;
        case BorderEdge::Bottom:
            return borders_data.bottom;
        default: // BorderEdge::Left:
            return borders_data.left;
        }
    }();

    float width = border_data.width;
    if (width <= 0)
        return;

    auto color = border_data.color;
    auto border_style = border_data.line_style;
    int int_width = max((int)width, 1);

    struct Points {
        Gfx::IntPoint p1;
        Gfx::IntPoint p2;
    };

    auto points_for_edge = [](BorderEdge edge, Gfx::IntRect const& rect) -> Points {
        switch (edge) {
        case BorderEdge::Top:
            return { rect.top_left(), rect.top_right() };
        case BorderEdge::Right:
            return { rect.top_right(), rect.bottom_right() };
        case BorderEdge::Bottom:
            return { rect.bottom_left(), rect.bottom_right() };
        default: // Edge::Left
            return { rect.top_left(), rect.bottom_left() };
        }
    };

    auto [p1, p2] = points_for_edge(edge, rect);

    if (border_style == CSS::LineStyle::Inset) {
        auto top_left_color = Color::from_rgb(0x5a5a5a);
        auto bottom_right_color = Color::from_rgb(0x888888);
        color = (edge == BorderEdge::Left || edge == BorderEdge::Top) ? top_left_color : bottom_right_color;
    } else if (border_style == CSS::LineStyle::Outset) {
        auto top_left_color = Color::from_rgb(0x888888);
        auto bottom_right_color = Color::from_rgb(0x5a5a5a);
        color = (edge == BorderEdge::Left || edge == BorderEdge::Top) ? top_left_color : bottom_right_color;
    }

    auto gfx_line_style = Gfx::Painter::LineStyle::Solid;
    if (border_style == CSS::LineStyle::Dotted)
        gfx_line_style = Gfx::Painter::LineStyle::Dotted;
    if (border_style == CSS::LineStyle::Dashed)
        gfx_line_style = Gfx::Painter::LineStyle::Dashed;

    if (gfx_line_style != Gfx::Painter::LineStyle::Solid) {
        switch (edge) {
        case BorderEdge::Top:
            p1.translate_by(int_width / 2, int_width / 2);
            p2.translate_by(-int_width / 2, int_width / 2);
            break;
        case BorderEdge::Right:
            p1.translate_by(-int_width / 2, int_width / 2);
            p2.translate_by(-int_width / 2, -int_width / 2);
            break;
        case BorderEdge::Bottom:
            p1.translate_by(int_width / 2, -int_width / 2);
            p2.translate_by(-int_width / 2, -int_width / 2);
            break;
        case BorderEdge::Left:
            p1.translate_by(int_width / 2, int_width / 2);
            p2.translate_by(int_width / 2, -int_width / 2);
            break;
        }
        context.painter().draw_line({ (int)p1.x(), (int)p1.y() }, { (int)p2.x(), (int)p2.y() }, color, int_width, gfx_line_style);
        return;
    }

    auto draw_line = [&](auto& p1, auto& p2) {
        context.painter().draw_line({ (int)p1.x(), (int)p1.y() }, { (int)p2.x(), (int)p2.y() }, color, 1, gfx_line_style);
    };

    float p1_step = 0;
    float p2_step = 0;

    bool has_top_left_radius = bool(border_radii_data.top_left);
    bool has_top_right_radius = bool(border_radii_data.top_right);
    bool has_bottom_left_radius = bool(border_radii_data.bottom_left);
    bool has_bottom_right_radius = bool(border_radii_data.bottom_right);

    switch (edge) {
    case BorderEdge::Top:
        p1_step = has_top_left_radius ? 0 : borders_data.left.width / (float)int_width;
        p2_step = has_top_right_radius ? 0 : borders_data.right.width / (float)int_width;
        for (int i = 0; i < int_width; ++i) {
            draw_line(p1, p2);
            p1.translate_by(p1_step, 1);
            p2.translate_by(-p2_step, 1);
        }
        break;
    case BorderEdge::Right:
        p1_step = has_top_right_radius ? 0 : borders_data.top.width / (float)int_width;
        p2_step = has_bottom_right_radius ? 0 : borders_data.bottom.width / (float)int_width;
        for (int i = int_width - 1; i >= 0; --i) {
            draw_line(p1, p2);
            p1.translate_by(-1, p1_step);
            p2.translate_by(-1, -p2_step);
        }
        break;
    case BorderEdge::Bottom:
        p1_step = has_bottom_left_radius ? 0 : borders_data.left.width / (float)int_width;
        p2_step = has_bottom_right_radius ? 0 : borders_data.right.width / (float)int_width;
        for (int i = int_width - 1; i >= 0; --i) {
            draw_line(p1, p2);
            p1.translate_by(p1_step, -1);
            p2.translate_by(-p2_step, -1);
        }
        break;
    case BorderEdge::Left:
        p1_step = has_top_left_radius ? 0 : borders_data.top.width / (float)int_width;
        p2_step = has_bottom_left_radius ? 0 : borders_data.bottom.width / (float)int_width;
        for (int i = 0; i < int_width; ++i) {
            draw_line(p1, p2);
            p1.translate_by(1, p1_step);
            p2.translate_by(1, -p2_step);
        }
        break;
    }
}

void paint_all_borders(PaintContext& context, Gfx::FloatRect const& bordered_rect, BorderRadiiData const& border_radii_data, BordersData const& borders_data)
{
    Gfx::IntRect border_rect = bordered_rect.to_rounded<int>();

    auto top_left = border_radii_data.top_left.as_corner();
    auto top_right = border_radii_data.top_right.as_corner();
    auto bottom_right = border_radii_data.bottom_right.as_corner();
    auto bottom_left = border_radii_data.bottom_left.as_corner();

    Gfx::IntRect top_border_rect = {
        border_rect.x() + top_left.horizontal_radius,
        border_rect.y(),
        border_rect.width() - top_left.horizontal_radius - top_right.horizontal_radius,
        border_rect.height()
    };
    Gfx::IntRect right_border_rect = {
        border_rect.x(),
        border_rect.y() + top_right.vertical_radius,
        border_rect.width(),
        border_rect.height() - top_right.vertical_radius - bottom_right.vertical_radius
    };
    Gfx::IntRect bottom_border_rect = {
        border_rect.x() + bottom_left.horizontal_radius,
        border_rect.y(),
        border_rect.width() - bottom_left.horizontal_radius - bottom_right.horizontal_radius,
        border_rect.height()
    };
    Gfx::IntRect left_border_rect = {
        border_rect.x(),
        border_rect.y() + top_left.vertical_radius,
        border_rect.width(),
        border_rect.height() - top_left.vertical_radius - bottom_left.vertical_radius
    };

    auto border_color_no_alpha = borders_data.top.color;
    border_color_no_alpha.set_alpha(255);

    Painting::paint_border(context, Painting::BorderEdge::Top, top_border_rect, border_radii_data, borders_data);
    Painting::paint_border(context, Painting::BorderEdge::Right, right_border_rect, border_radii_data, borders_data);
    Painting::paint_border(context, Painting::BorderEdge::Bottom, bottom_border_rect, border_radii_data, borders_data);
    Painting::paint_border(context, Painting::BorderEdge::Left, left_border_rect, border_radii_data, borders_data);

    if (borders_data.top.width <= 0 && borders_data.right.width <= 0 && borders_data.left.width <= 0 && borders_data.bottom.width <= 0)
        return;

    auto int_width = [&](auto value) -> int {
        return ceil(value);
    };

    // Cache a small bitmap, just large enough to fit the corners (without the inner rectangle)
    auto expand_width = abs(int_width(borders_data.left.width) - int_width(borders_data.right.width));
    auto expand_height = abs(int_width(borders_data.top.width) - int_width(borders_data.bottom.width));
    Gfx::IntRect corner_mask_rect {
        0, 0,
        max(
            top_left.horizontal_radius + top_right.horizontal_radius + expand_width,
            bottom_left.horizontal_radius + bottom_right.horizontal_radius + expand_height),
        max(
            top_left.vertical_radius + bottom_left.vertical_radius + expand_width,
            top_right.vertical_radius + bottom_right.vertical_radius + expand_height)
    };
    auto allocate_mask_bitmap = [&]{
        return MUST(Gfx::Bitmap::try_create(Gfx::BitmapFormat::BGRA8888, corner_mask_rect.size()));
    };
    static auto corner_bitmap = allocate_mask_bitmap();

    Gfx::Painter painter { corner_bitmap };
    if (corner_bitmap->rect().contains(corner_mask_rect)) {
        painter.clear_rect(corner_mask_rect, Gfx::Color());
    } else {
        corner_bitmap = allocate_mask_bitmap();
        painter = Gfx::Painter { corner_bitmap };
    }

    Gfx::AntiAliasingPainter aa_painter { painter };

    aa_painter.fill_rect_with_rounded_corners(corner_mask_rect, border_color_no_alpha, top_left, top_right, bottom_right, bottom_left, Gfx::AntiAliasingPainter::BlendMode::Normal);


    auto inner_corner_mask_rect = corner_mask_rect.shrunken(
        int_width(borders_data.top.width),
        int_width(borders_data.right.width),
        int_width(borders_data.bottom.width),
        int_width(borders_data.left.width));

    auto inner_top_left = top_left;
    auto inner_top_right = top_right;
    auto inner_bottom_right = bottom_right;
    auto inner_bottom_left = bottom_left;

    inner_top_left.horizontal_radius = max(0, inner_top_left.horizontal_radius - int_width(borders_data.left.width));
    inner_top_left.vertical_radius = max(0, inner_top_left.vertical_radius - int_width(borders_data.top.width));

    inner_top_right.horizontal_radius = max(0, inner_top_right.horizontal_radius - int_width(borders_data.right.width));
    inner_top_right.vertical_radius = max(0, inner_top_right.vertical_radius - int_width(borders_data.top.width));

    inner_bottom_right.horizontal_radius = max(0, inner_bottom_right.horizontal_radius - int_width(borders_data.right.width));
    inner_bottom_right.vertical_radius = max(0, inner_bottom_right.vertical_radius - int_width(borders_data.bottom.width));

    inner_bottom_left.horizontal_radius = max(0, inner_bottom_left.horizontal_radius - int_width(borders_data.left.width));
    inner_bottom_left.vertical_radius = max(0, inner_bottom_left.vertical_radius - int_width(borders_data.bottom.width));

    aa_painter.fill_rect_with_rounded_corners(inner_corner_mask_rect, border_color_no_alpha, inner_top_left, inner_top_right, inner_bottom_right, inner_bottom_left, Gfx::AntiAliasingPainter::BlendMode::AlphaSubtract);

    if (top_left)
        context.painter().blit(border_rect.top_left(), corner_bitmap, top_left.as_rect(), borders_data.top.color.alpha()/255.);

    if (top_right)
        context.painter().blit(border_rect.top_right().translated(-top_right.horizontal_radius + 1, 0), corner_bitmap, top_right.as_rect().translated(corner_mask_rect.width() - top_right.horizontal_radius, 0) , borders_data.top.color.alpha()/255.);

    if (bottom_right)
        context.painter().blit(border_rect.bottom_right().translated(-bottom_right.horizontal_radius + 1, -bottom_right.vertical_radius + 1), corner_bitmap, bottom_right.as_rect().translated(corner_mask_rect.width() - bottom_right.horizontal_radius, corner_mask_rect.height() - bottom_right.vertical_radius), borders_data.top.color.alpha()/255.);

    if (bottom_left)
        context.painter().blit(border_rect.bottom_left().translated(0, -bottom_left.vertical_radius + 1), corner_bitmap, bottom_left.as_rect().translated(0, corner_mask_rect.height() - bottom_left.vertical_radius), borders_data.top.color.alpha()/255.);

    // context.painter().blit(corner_mask_rect.centered_within(border_rect).location(),
    //     corner_bitmap, corner_mask_rect);
}

}
