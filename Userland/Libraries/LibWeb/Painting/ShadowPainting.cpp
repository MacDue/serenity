/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2021-2022, Sam Atkins <atkinssj@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/DisjointRectSet.h>
#include <LibGfx/Filters/FastBoxBlurFilter.h>
#include <LibGfx/Painter.h>
#include <LibWeb/Layout/LineBoxFragment.h>
#include <LibWeb/Painting/PaintContext.h>
#include <LibWeb/Painting/ShadowPainting.h>
#include <LibWeb/Painting/BorderPainting.h>

namespace Web::Painting {

void paint_box_shadow(PaintContext& context, Gfx::IntRect const& content_rect, BorderRadiiData const& border_radii, Vector<ShadowData> const& box_shadow_layers)
{
    if (box_shadow_layers.is_empty())
        return;

    auto& painter = context.painter();

    [[maybe_unused]] auto top_left_corner = border_radii.top_left.as_corner();
    [[maybe_unused]] auto top_right_corner = border_radii.top_right.as_corner();
    [[maybe_unused]] auto bottom_right_corner = border_radii.bottom_right.as_corner();
    [[maybe_unused]] auto bottom_left_corner = border_radii.bottom_left.as_corner();

    // Note: Box-shadow layers are ordered front-to-back, so we paint them in reverse
    for (int layer_index = box_shadow_layers.size() - 1; layer_index >= 0; layer_index--) {
        auto& box_shadow_data = box_shadow_layers[layer_index];
        // FIXME: Paint inset shadows.
        if (box_shadow_data.placement != ShadowPlacement::Outer)
            continue;

        // FIXME: Account for rounded corners.

        // auto fill_rect_masked = [](auto& painter, auto fill_rect, auto mask_rect, auto color) {
        //     Gfx::DisjointRectSet rect_set;
        //     rect_set.add(fill_rect);
        //     auto shattered = rect_set.shatter(mask_rect);
        //     for (auto& rect : shattered.rects())
        //         painter.fill_rect(rect, color);
        // };

        // If there's no blurring, we can save a lot of effort.
        if (box_shadow_data.blur_radius == 0) {
            // fill_rect_masked(painter, content_rect.inflated(box_shadow_data.spread_distance, box_shadow_data.spread_distance, box_shadow_data.spread_distance, box_shadow_data.spread_distance).translated(box_shadow_data.offset_x, box_shadow_data.offset_y), content_rect, box_shadow_data.color);
            continue;
        }

        auto expansion = box_shadow_data.spread_distance - (box_shadow_data.blur_radius * 2);
        Gfx::IntRect solid_rect = {
            content_rect.x() + box_shadow_data.offset_x - expansion,
            content_rect.y() + box_shadow_data.offset_y - expansion,
            content_rect.width() + 2 * expansion,
            content_rect.height() + 2 * expansion
        };
        painter.fill_rect(solid_rect, box_shadow_data.color);

        // Calculating and blurring the box-shadow full size is expensive, and wasteful - aside from the corners,
        // all vertical strips of the shadow are identical, and the same goes for horizontal ones.
        // So instead, we generate a shadow bitmap that is just large enough to include the corners and 1px of
        // non-corner, and then we repeatedly blit sections of it. This is similar to a NinePatch on Android.
        auto double_radius = box_shadow_data.blur_radius * 2;

        auto shadow_bitmap_rect = Gfx::IntRect (
            0, 0,
            max(
                top_left_corner.horizontal_radius + top_right_corner.horizontal_radius,
                bottom_left_corner.horizontal_radius + bottom_right_corner.horizontal_radius) + 1 + double_radius * 2,
            max(
                top_left_corner.vertical_radius + bottom_left_corner.vertical_radius,
                top_right_corner.vertical_radius + bottom_right_corner.vertical_radius) + 1 + double_radius * 2
        );

        auto corner_size = box_shadow_data.blur_radius * 4;

        [[maybe_unused]]  auto top_left_corner_rect = Gfx::IntRect {
            0, 0,
            top_left_corner.horizontal_radius + double_radius,
            top_left_corner.vertical_radius + double_radius
        };
        [[maybe_unused]]  auto top_right_corner_rect = Gfx::IntRect {
            shadow_bitmap_rect.width() - (top_right_corner.horizontal_radius + double_radius), 0,
            top_right_corner.horizontal_radius + double_radius,
            top_right_corner.vertical_radius + double_radius
        };
        [[maybe_unused]] auto bottom_right_corner_rect = Gfx::IntRect  {
            shadow_bitmap_rect.width() - (bottom_right_corner.horizontal_radius + double_radius),
            shadow_bitmap_rect.height() - (bottom_right_corner.vertical_radius + double_radius),
            bottom_right_corner.horizontal_radius + double_radius,
            bottom_right_corner.vertical_radius + double_radius
        };
        [[maybe_unused]] auto bottom_left_corner_rect = Gfx::IntRect  {
            0, shadow_bitmap_rect.height() - (bottom_left_corner.vertical_radius + double_radius),
            bottom_left_corner.horizontal_radius + double_radius,
            bottom_left_corner.vertical_radius + double_radius
        };

        Gfx::IntRect left_edge_rect { 0, top_left_corner_rect.height(), corner_size, 1 };
        Gfx::IntRect right_edge_rect { shadow_bitmap_rect.width() - corner_size, top_right_corner_rect.height(), corner_size, 1 };
        Gfx::IntRect top_edge_rect { top_left_corner_rect.width(), 0, 1, corner_size };
        Gfx::IntRect bottom_edge_rect { bottom_left_corner_rect.width(), shadow_bitmap_rect.height() - corner_size, 1, corner_size };

        auto shadows_bitmap = Gfx::Bitmap::try_create(Gfx::BitmapFormat::BGRA8888, shadow_bitmap_rect.size());
        if (shadows_bitmap.is_error()) {
            dbgln("Unable to allocate temporary bitmap for box-shadow rendering: {}", shadows_bitmap.error());
            return;
        }
        auto shadow_bitmap = shadows_bitmap.release_value();
        Gfx::Painter corner_painter { *shadow_bitmap };
        Gfx::AntiAliasingPainter aa_corner_painter { corner_painter };

        aa_corner_painter.fill_rect_with_rounded_corners(shadow_bitmap_rect.shrunken(double_radius, double_radius, double_radius, double_radius), box_shadow_data.color, top_left_corner, top_right_corner, bottom_right_corner, bottom_left_corner);
        Gfx::FastBoxBlurFilter filter(*shadow_bitmap);
        filter.apply_three_passes(box_shadow_data.blur_radius);


        auto left_start = solid_rect.left() - corner_size;
        auto right_start = solid_rect.left() + solid_rect.width();
        auto top_start = solid_rect.top() - corner_size;
        auto bottom_start = solid_rect.top() + solid_rect.height();

        // FIXME: Painter only lets us define a clip-rect which discards drawing outside of it, whereas here we want
        //        a rect which discards drawing inside it. So, we run the draw operations 4 times with clip-rects
        //        covering each side of the content_rect exactly once.
        auto paint_shadow = [&](Gfx::IntRect clip_rect) {
            painter.save();
            (void) clip_rect;
            // painter.add_clip_rect(clip_rect);

            // Paint corners
            // painter.blit({ left_start, top_start }, shadow_bitmap, corner_rect);
            // painter.blit({ right_start, top_start }, shadow_bitmap, corner_rect.translated(corner_rect.width() + 1, 0));
            // painter.blit({ left_start, bottom_start }, shadow_bitmap, corner_rect.translated(0, corner_rect.height() + 1));
            // painter.blit({ right_start, bottom_start }, shadow_bitmap, corner_rect.translated(corner_rect.width() + 1, corner_rect.height() + 1));

            painter.blit(solid_rect.top_left().translated(-double_radius*2,-double_radius*2), shadow_bitmap, top_left_corner_rect);
            painter.blit(solid_rect.top_right().translated(-top_right_corner.horizontal_radius + 1 + double_radius, -double_radius*2), shadow_bitmap, top_right_corner_rect);
            painter.blit(solid_rect.bottom_left().translated(-double_radius*2, -bottom_left_corner.vertical_radius + 1 + double_radius), shadow_bitmap, bottom_left_corner_rect);
            painter.blit(solid_rect.bottom_right().translated(-bottom_right_corner.horizontal_radius + 1 + double_radius, -bottom_right_corner.vertical_radius + 1 + double_radius), shadow_bitmap, bottom_right_corner_rect);

            // Horizontal edges
            for (auto x = solid_rect.left() + (bottom_left_corner.horizontal_radius - double_radius); x <= solid_rect.right() - (bottom_right_corner.horizontal_radius - double_radius); ++x)
                painter.blit({ x, bottom_start }, shadow_bitmap, bottom_edge_rect);

            for (auto x = solid_rect.left() + (top_left_corner.horizontal_radius - double_radius); x <= solid_rect.right() - (top_right_corner.horizontal_radius - double_radius); ++x)
                painter.blit({ x, top_start }, shadow_bitmap, top_edge_rect);

            // Vertical edges
            for (auto y = solid_rect.top() + (top_right_corner.vertical_radius - double_radius); y <= solid_rect.bottom() - (bottom_right_corner.vertical_radius - double_radius); ++y)
                painter.blit({ right_start, y }, shadow_bitmap, right_edge_rect);

            for (auto y = solid_rect.top() + (top_left_corner.vertical_radius - double_radius); y <= solid_rect.bottom() - (bottom_left_corner.vertical_radius - double_radius); ++y)
                painter.blit({ left_start, y }, shadow_bitmap, left_edge_rect);

            painter.restore();
        };

        // Everything above content_rect, including sides
        paint_shadow({ 0, 0, painter.target()->width(), content_rect.top() });

        // // Everything below content_rect, including sides
        // paint_shadow({ 0, content_rect.bottom() + 1, painter.target()->width(), painter.target()->height() });

        // // Everything directly to the left of content_rect
        // paint_shadow({ 0, content_rect.top(), content_rect.left(), content_rect.height() });

        // // Everything directly to the right of content_rect
        // paint_shadow({ content_rect.right() + 1, content_rect.top(), painter.target()->width(), content_rect.height() });

            // painter.fill_rect(corner_rect.translated(solid_rect.location()), Gfx::Color::NamedColor::Red);
    }
}

void paint_text_shadow(PaintContext& context, Layout::LineBoxFragment const& fragment, Vector<ShadowData> const& shadow_layers)
{
    if (shadow_layers.is_empty())
        return;

    auto& painter = context.painter();

    // Note: Box-shadow layers are ordered front-to-back, so we paint them in reverse
    for (auto& layer : shadow_layers.in_reverse()) {

        // Space around the painted text to allow it to blur.
        // FIXME: Include spread in this once we use that.
        auto margin = layer.blur_radius * 2;
        Gfx::IntRect text_rect {
            margin, margin,
            static_cast<int>(ceilf(fragment.width())),
            static_cast<int>(ceilf(fragment.height()))
        };
        Gfx::IntRect bounding_rect {
            0, 0,
            text_rect.width() + margin + margin,
            text_rect.height() + margin + margin
        };
        // FIXME: Figure out the maximum bitmap size for all shadows and then allocate it once and reuse it?
        auto maybe_shadow_bitmap = Gfx::Bitmap::try_create(Gfx::BitmapFormat::BGRA8888, bounding_rect.size());
        if (maybe_shadow_bitmap.is_error()) {
            dbgln("Unable to allocate temporary bitmap for box-shadow rendering: {}", maybe_shadow_bitmap.error());
            return;
        }
        auto shadow_bitmap = maybe_shadow_bitmap.release_value();

        Gfx::Painter shadow_painter { *shadow_bitmap };
        shadow_painter.set_font(context.painter().font());
        // FIXME: "Spread" the shadow somehow.
        Gfx::FloatPoint baseline_start(text_rect.x(), text_rect.y() + fragment.baseline());
        shadow_painter.draw_text_run(baseline_start, Utf8View(fragment.text()), context.painter().font(), layer.color);

        // Blur
        Gfx::FastBoxBlurFilter filter(*shadow_bitmap);
        filter.apply_three_passes(layer.blur_radius);

        auto draw_rect = Gfx::enclosing_int_rect(fragment.absolute_rect());
        Gfx::IntPoint draw_location {
            draw_rect.x() + layer.offset_x - margin,
            draw_rect.y() + layer.offset_y - margin
        };
        painter.blit(draw_location, *shadow_bitmap, bounding_rect);
    }
}

}
