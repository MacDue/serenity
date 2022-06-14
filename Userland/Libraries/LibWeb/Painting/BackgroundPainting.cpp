/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2021, Sam Atkins <atkinssj@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/AntiAliasingPainter.h>
#include <LibGfx/Painter.h>
#include <LibWeb/Layout/InitialContainingBlock.h>
#include <LibWeb/Layout/Node.h>
#include <LibWeb/Painting/BackgroundPainting.h>
#include <LibWeb/Painting/PaintContext.h>

namespace Web::Painting {

// https://www.w3.org/TR/css-backgrounds-3/#backgrounds
void paint_background(PaintContext& context, Layout::NodeWithStyleAndBoxModelMetrics const& layout_node, Gfx::FloatRect const& border_rect, Color background_color, Vector<CSS::BackgroundLayerData> const* background_layers, BorderRadiiData const& border_radii)
{
    auto& painter = context.painter();

    auto get_box = [&](CSS::BackgroundBox box) {
        auto box_rect = border_rect;
        switch (box) {
        case CSS::BackgroundBox::ContentBox: {
            auto& padding = layout_node.box_model().padding;
            box_rect.shrink(padding.top, padding.right, padding.bottom, padding.left);
            [[fallthrough]];
        }
        case CSS::BackgroundBox::PaddingBox: {
            auto& border = layout_node.box_model().border;
            box_rect.shrink(border.top, border.right, border.bottom, border.left);
            [[fallthrough]];
        }
        case CSS::BackgroundBox::BorderBox:
        default:
            return box_rect;
        }
    };

    auto color_rect = border_rect;
    if (background_layers && !background_layers->is_empty())
        color_rect = get_box(background_layers->last().clip);

    Gfx::AntiAliasingPainter aa_painter { painter };
    aa_painter.fill_rect_with_rounded_corners(color_rect.to_rounded<int>(),
        background_color, border_radii.top_left.as_corner(), border_radii.top_right.as_corner(), border_radii.bottom_right.as_corner(), border_radii.bottom_left.as_corner());

    if (!background_layers)
        return;

    // Note: Background layers are ordered front-to-back, so we paint them in reverse
    for (int layer_index = background_layers->size() - 1; layer_index >= 0; layer_index--) {
        auto& layer = background_layers->at(layer_index);
        // TODO: Gradients!
        if (!layer.image || !layer.image->bitmap())
            continue;
        auto& image = *layer.image->bitmap();

        // Clip
        auto clip_rect = get_box(layer.clip);
        painter.save();
        painter.add_clip_rect(clip_rect.to_rounded<int>());

        Gfx::FloatRect background_positioning_area;

        // Attachment and Origin
        switch (layer.attachment) {
        case CSS::BackgroundAttachment::Fixed:
            background_positioning_area = layout_node.root().browsing_context().viewport_rect().to_type<float>();
            break;
        case CSS::BackgroundAttachment::Local:
        case CSS::BackgroundAttachment::Scroll:
            background_positioning_area = get_box(layer.origin);
            break;
        }

        // Size
        Gfx::FloatRect image_rect;
        switch (layer.size_type) {
        case CSS::BackgroundSize::Contain: {
            float max_width_ratio = background_positioning_area.width() / image.width();
            float max_height_ratio = background_positioning_area.height() / image.height();
            float ratio = min(max_width_ratio, max_height_ratio);
            image_rect.set_size(image.width() * ratio, image.height() * ratio);
            break;
        }
        case CSS::BackgroundSize::Cover: {
            float max_width_ratio = background_positioning_area.width() / image.width();
            float max_height_ratio = background_positioning_area.height() / image.height();
            float ratio = max(max_width_ratio, max_height_ratio);
            image_rect.set_size(image.width() * ratio, image.height() * ratio);
            break;
        }
        case CSS::BackgroundSize::LengthPercentage: {
            float width;
            float height;
            bool x_is_auto = layer.size_x.is_length() && layer.size_x.length().is_auto();
            bool y_is_auto = layer.size_y.is_length() && layer.size_y.length().is_auto();
            if (x_is_auto && y_is_auto) {
                width = image.width();
                height = image.height();
            } else if (x_is_auto) {
                height = layer.size_y.resolved(layout_node, CSS::Length::make_px(background_positioning_area.height())).to_px(layout_node);
                width = image.width() * (height / image.height());
            } else if (y_is_auto) {
                width = layer.size_x.resolved(layout_node, CSS::Length::make_px(background_positioning_area.width())).to_px(layout_node);
                height = image.height() * (width / image.width());
            } else {
                width = layer.size_x.resolved(layout_node, CSS::Length::make_px(background_positioning_area.width())).to_px(layout_node);
                height = layer.size_y.resolved(layout_node, CSS::Length::make_px(background_positioning_area.height())).to_px(layout_node);
            }

            image_rect.set_size(width, height);
            break;
        }
        }

        // If background-repeat is round for one (or both) dimensions, there is a second step.
        // The UA must scale the image in that dimension (or both dimensions) so that it fits a
        // whole number of times in the background positioning area.
        if (layer.repeat_x == CSS::Repeat::Round || layer.repeat_y == CSS::Repeat::Round) {
            // If X ≠ 0 is the width of the image after step one and W is the width of the
            // background positioning area, then the rounded width X' = W / round(W / X)
            // where round() is a function that returns the nearest natural number
            // (integer greater than zero).
            if (layer.repeat_x == CSS::Repeat::Round) {
                image_rect.set_width(background_positioning_area.width() / background_positioning_area.width() / image_rect.width());
            }
            if (layer.repeat_y == CSS::Repeat::Round) {
                image_rect.set_height(background_positioning_area.height() / background_positioning_area.height() / image_rect.height());
            }

            // If background-repeat is round for one dimension only and if background-size is auto
            // for the other dimension, then there is a third step: that other dimension is scaled
            // so that the original aspect ratio is restored.
            if (layer.repeat_x != layer.repeat_y) {
                if (layer.size_x.is_length() && layer.size_x.length().is_auto()) {
                    image_rect.set_width(image.width() * (image_rect.height() / image.height()));
                }
                if (layer.size_y.is_length() && layer.size_y.length().is_auto()) {
                    image_rect.set_height(image.height() * (image_rect.width() / image.width()));
                }
            }
        }

        float space_x = background_positioning_area.width() - image_rect.width();
        float space_y = background_positioning_area.height() - image_rect.height();

        // Position
        float offset_x = layer.position_offset_x.resolved(layout_node, CSS::Length::make_px(space_x)).to_px(layout_node);
        if (layer.position_edge_x == CSS::PositionEdge::Right) {
            image_rect.set_right_without_resize(background_positioning_area.right() - offset_x);
        } else {
            image_rect.set_left(background_positioning_area.left() + offset_x);
        }

        float offset_y = layer.position_offset_y.resolved(layout_node, CSS::Length::make_px(space_y)).to_px(layout_node);
        if (layer.position_edge_y == CSS::PositionEdge::Bottom) {
            image_rect.set_bottom_without_resize(background_positioning_area.bottom() - offset_y);
        } else {
            image_rect.set_top(background_positioning_area.top() + offset_y);
        }

        // Repetition
        bool repeat_x = false;
        bool repeat_y = false;
        float x_step = 0;
        float y_step = 0;

        switch (layer.repeat_x) {
        case CSS::Repeat::Round:
            x_step = image_rect.width();
            repeat_x = true;
            break;
        case CSS::Repeat::Space: {
            int whole_images = background_positioning_area.width() / image_rect.width();
            if (whole_images <= 1) {
                x_step = image_rect.width();
                repeat_x = false;
            } else {
                float space = fmodf(background_positioning_area.width(), image_rect.width());
                x_step = image_rect.width() + ((float)space / (float)(whole_images - 1));
                repeat_x = true;
            }
            break;
        }
        case CSS::Repeat::Repeat:
            x_step = image_rect.width();
            repeat_x = true;
            break;
        case CSS::Repeat::NoRepeat:
            repeat_x = false;
            break;
        }
        // Move image_rect to the left-most tile position that is still visible
        if (repeat_x && image_rect.x() > clip_rect.x()) {
            auto x_delta = floorf(x_step * ceilf((image_rect.x() - clip_rect.x()) / x_step));
            image_rect.set_x(image_rect.x() - x_delta);
        }

        switch (layer.repeat_y) {
        case CSS::Repeat::Round:
            y_step = image_rect.height();
            repeat_y = true;
            break;
        case CSS::Repeat::Space: {
            int whole_images = background_positioning_area.height() / image_rect.height();
            if (whole_images <= 1) {
                y_step = image_rect.height();
                repeat_y = false;
            } else {
                float space = fmodf(background_positioning_area.height(), image_rect.height());
                y_step = image_rect.height() + ((float)space / (float)(whole_images - 1));
                repeat_y = true;
            }
            break;
        }
        case CSS::Repeat::Repeat:
            y_step = image_rect.height();
            repeat_y = true;
            break;
        case CSS::Repeat::NoRepeat:
            repeat_y = false;
            break;
        }
        // Move image_rect to the top-most tile position that is still visible
        if (repeat_y && image_rect.y() > clip_rect.y()) {
            auto y_delta = floorf(y_step * ceilf((image_rect.y() - clip_rect.y()) / y_step));
            image_rect.set_y(image_rect.y() - y_delta);
        }

        auto paint_image_background = [&]{
            float initial_image_x = image_rect.x();
            float image_y = image_rect.y();
            while (image_y < clip_rect.bottom()) {
                image_rect.set_y(image_y);

                float image_x = initial_image_x;
                while (image_x < clip_rect.right()) {
                    image_rect.set_x(image_x);
                    painter.draw_scaled_bitmap(image_rect.to_rounded<int>(), image, image.rect(), 1.0f, Gfx::Painter::ScalingMode::BilinearBlend);
                    if (!repeat_x)
                        break;
                    image_x += x_step;
                }

                if (!repeat_y)
                    break;
                image_y += y_step;
            }
        };

        if (border_radii.has_any_radius()) {
            Gfx::IntRect int_border_rect = border_rect.to_rounded<int>();

            auto top_left = border_radii.top_left.as_corner();
            auto top_right = border_radii.top_right.as_corner();
            auto bottom_right = border_radii.bottom_right.as_corner();
            auto bottom_left =  border_radii.bottom_left.as_corner();

            Gfx::IntRect corner_rect {
                0, 0,
                max(
                    top_left.horizontal_radius + top_right.horizontal_radius,
                    bottom_left.horizontal_radius + bottom_right.horizontal_radius),
                max(
                    top_left.vertical_radius + bottom_left.vertical_radius,
                    top_right.vertical_radius + bottom_right.vertical_radius)
            };

            auto allocate_corner_bitmap = [&]() -> RefPtr<Gfx::Bitmap> {
                auto bitmap = Gfx::Bitmap::try_create(Gfx::BitmapFormat::BGRA8888, corner_rect.size());
                if (!bitmap.is_error())
                    return bitmap.release_value();
                return nullptr;
            };
            static thread_local auto corner_bitmap = allocate_corner_bitmap();

            Gfx::Painter corner_painter = ({
                Optional<Gfx::Painter> painter;
                if (corner_bitmap && corner_bitmap->rect().contains(corner_rect)) {
                    painter = Gfx::Painter { *corner_bitmap };
                    painter->clear_rect(corner_rect, Gfx::Color());
                } else {
                    corner_bitmap = allocate_corner_bitmap();
                    if (!corner_bitmap)
                        return dbgln("Failed to background image corner bitmap with size {}", corner_rect.size());
                    painter = Gfx::Painter { *corner_bitmap };
                }
                *painter;
            });

            auto top_left_corner_page_location = int_border_rect.top_left();
            auto top_right_corner_page_location = int_border_rect.top_right().translated(-top_right.horizontal_radius + 1, 0);
            auto bottom_right_corner_page_location = int_border_rect.bottom_right().translated(-bottom_right.horizontal_radius + 1, -bottom_right.vertical_radius + 1);
            auto bottom_left_corner_page_location = int_border_rect.bottom_left().translated(0, -bottom_left.vertical_radius + 1);

            Gfx::IntPoint top_left_bitmap_location{0, 0};
            Gfx::IntPoint top_right_bitmap_location{corner_bitmap->width() - top_right.horizontal_radius, 0};
            Gfx::IntPoint bottom_right_bitmap_location{corner_bitmap->width() - bottom_right.horizontal_radius, corner_bitmap->height() - bottom_right.vertical_radius};
            Gfx::IntPoint bottom_left_bitmap_location{0, corner_bitmap->height() - bottom_left.vertical_radius};

            auto copy_page_masked = [&](auto const & mask_src, auto const & page_location) {
                for (int row = 0; row < mask_src.height(); ++row) {
                    for (int col = 0; col < mask_src.width(); ++col) {
                        auto corner_location = mask_src.location().translated(col, row);
                        auto mask_pixel = corner_bitmap->get_pixel(corner_location);
                        u8 mask_alpha = ~mask_pixel.alpha();
                        Color final_pixel{};
                        if (mask_alpha > 0) {
                            auto page_pixel = painter.get_pixel(page_location.translated(col, row));
                            if (page_pixel.has_value())
                                final_pixel = page_pixel.value().with_alpha(mask_alpha);
                        }
                        corner_bitmap->set_pixel(corner_location, final_pixel);
                    }
                }
            };

            // Generate a mask for the corner:
            Gfx::AntiAliasingPainter corner_aa_painter { corner_painter };
            corner_aa_painter.fill_rect_with_rounded_corners(corner_rect, Color::NamedColor::Black, top_left, top_right, bottom_right, bottom_left);

            // Copy the pixels under the corner mask:
            if (top_left)
                copy_page_masked(top_left.as_rect().translated(top_left_bitmap_location), top_left_corner_page_location);
            if (top_right)
                copy_page_masked(top_right.as_rect().translated(top_right_bitmap_location), top_right_corner_page_location);
            if (bottom_right)
                copy_page_masked(bottom_right.as_rect().translated(bottom_right_bitmap_location), bottom_right_corner_page_location);
            if (bottom_left)
                copy_page_masked(bottom_left.as_rect().translated(bottom_left_bitmap_location), bottom_left_corner_page_location);

            // Paint the image background:
            paint_image_background();

            // Restore the corners:
            if (top_left)
                painter.blit(top_left_corner_page_location, *corner_bitmap, top_left.as_rect().translated(top_left_bitmap_location));
            if (top_right)
                painter.blit(top_right_corner_page_location, *corner_bitmap, top_right.as_rect().translated(top_right_bitmap_location));
            if (bottom_right)
                painter.blit(bottom_right_corner_page_location, *corner_bitmap, bottom_right.as_rect().translated(bottom_right_bitmap_location));
            if (bottom_left)
                painter.blit(bottom_left_corner_page_location, *corner_bitmap, bottom_left.as_rect().translated(bottom_left_bitmap_location));
        } else {
            paint_image_background();
        }

        painter.restore();
    }
}

}
