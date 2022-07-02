#include <LibGfx/ShadowPainter.h>
#include <LibGfx/Filters/StackBlurFilter.h>

namespace Gfx {

void ShadowPainter::paint_shadow_around(Gfx::IntRect const & rect, CornerRadius top_left, CornerRadius top_right, CornerRadius bottom_right, CornerRadius bottom_left, Infill infill) {

       // Calculating and blurring the box-shadow full size is expensive, and wasteful - aside from the corners,
        // all vertical strips of the shadow are identical, and the same goes for horizontal ones.
        // So instead, we generate a shadow bitmap that is just large enough to include the corners and 1px of
        // non-corner, and then we repeatedly blit sections of it. This is similar to a NinePatch on Android.
        auto double_radius = m_blur_radius * 2;
        auto blurred_edge_thickness = m_blur_radius * 4;

        auto default_corner_size = Gfx::IntSize { double_radius, double_radius };
        auto top_left_corner_size = top_left ? top_left.as_rect().size() : default_corner_size;
        auto top_right_corner_size = top_right ? top_right.as_rect().size() : default_corner_size;
        auto bottom_left_corner_size = bottom_left ? bottom_left.as_rect().size() : default_corner_size;
        auto bottom_right_corner_size = bottom_right ? bottom_right.as_rect().size() : default_corner_size;

        auto max_edge_width = non_blurred_shadow_rect.width() / 2;
        auto max_edge_height = non_blurred_shadow_rect.height() / 2;
        auto extra_edge_width = non_blurred_shadow_rect.width() % 2;
        auto extra_edge_height = non_blurred_shadow_rect.height() % 2;

        auto clip_corner_size = [&](auto& size, int x_bonus = 0, int y_bonus = 0) {
            size.set_width(min(size.width(), max_edge_width + x_bonus));
            size.set_height(min(size.height(), max_edge_height + y_bonus));
        };

        if (!top_left)
            clip_corner_size(top_left_corner_size, extra_edge_width, extra_edge_height);
        if (!top_right)
            clip_corner_size(top_right_corner_size, 0, extra_edge_height);
        if (!bottom_left)
            clip_corner_size(bottom_left_corner_size, extra_edge_width);
        if (!bottom_right)
            clip_corner_size(bottom_right_corner_size);

        auto shadow_bitmap_rect = Gfx::IntRect(
            0, 0,
            max(
                top_left_corner_size.width() + top_right_corner_size.width(),
                bottom_left_corner_size.width() + bottom_right_corner_size.width())
                + 1 + blurred_edge_thickness,
            max(
                top_left_corner_size.height() + bottom_left_corner_size.height(),
                top_right_corner_size.height() + bottom_right_corner_size.height())
                + 1 + blurred_edge_thickness);

        auto top_left_corner_rect = Gfx::IntRect {
            0, 0,
            top_left_corner_size.width() + double_radius,
            top_left_corner_size.height() + double_radius
        };
        auto top_right_corner_rect = Gfx::IntRect {
            shadow_bitmap_rect.width() - (top_right_corner_size.width() + double_radius), 0,
            top_right_corner_size.width() + double_radius,
            top_right_corner_size.height() + double_radius
        };
        auto bottom_right_corner_rect = Gfx::IntRect {
            shadow_bitmap_rect.width() - (bottom_right_corner_size.width() + double_radius),
            shadow_bitmap_rect.height() - (bottom_right_corner_size.height() + double_radius),
            bottom_right_corner_size.width() + double_radius,
            bottom_right_corner_size.height() + double_radius
        };
        auto bottom_left_corner_rect = Gfx::IntRect {
            0, shadow_bitmap_rect.height() - (bottom_left_corner_size.height() + double_radius),
            bottom_left_corner_size.width() + double_radius,
            bottom_left_corner_size.height() + double_radius
        };

        auto horizontal_edge_width = min(max_edge_height, double_radius) + double_radius;
        auto vertical_edge_width = min(max_edge_width, double_radius) + double_radius;
        auto horizontal_top_edge_width = min(max_edge_height + extra_edge_height, double_radius) + double_radius;
        auto vertical_left_edge_width = min(max_edge_width + extra_edge_width, double_radius) + double_radius;

        Gfx::IntRect left_edge_rect { 0, top_left_corner_rect.height(), vertical_left_edge_width, 1 };
        Gfx::IntRect right_edge_rect { shadow_bitmap_rect.width() - vertical_edge_width, top_right_corner_rect.height(), vertical_edge_width, 1 };
        Gfx::IntRect top_edge_rect { top_left_corner_rect.width(), 0, 1, horizontal_top_edge_width };
        Gfx::IntRect bottom_edge_rect { bottom_left_corner_rect.width(), shadow_bitmap_rect.height() - horizontal_edge_width, 1, horizontal_edge_width };


        if (!m_shadow_bitmap || m_shadow_bitmap->size() != shadow_bitmap_rect.size()) {
          auto shadow_bitmap = Gfx::Bitmap::try_create(Gfx::BitmapFormat::BGRA8888, shadow_bitmap_rect.size());
          if (shadow_bitmap.is_error()) {
              dbgln("Unable to allocate temporary bitmap for box-shadow rendering: {}", shadow_bitmap.error());
              return;
          }
          m_shadow_bitmap = shadow_bitmap.release_value();

          Gfx::Painter corner_painter { *m_shadow_bitmap };
          Gfx::AntiAliasingPainter aa_corner_painter { corner_painter };

          aa_corner_painter.fill_rect_with_rounded_corners(shadow_bitmap_rect.shrunken(double_radius, double_radius, double_radius, double_radius), box_shadow_data.color, top_left_shadow_corner, top_right_shadow_corner, bottom_right_shadow_corner, bottom_left_shadow_corner);
          Gfx::StackBlurFilter filter(*m_shadow_bitmap);
          filter.process_rgba(m_blur_radius, m_color);
        }

        auto paint_shadow_infill = [&] {
            if (!border_radii.has_any_radius())
                return painter.fill_rect(inner_bounding_rect, box_shadow_data.color);

            auto top_left_inner_width = top_left_corner_rect.width() - blurred_edge_thickness;
            auto top_left_inner_height = top_left_corner_rect.height() - blurred_edge_thickness;
            auto top_right_inner_width = top_right_corner_rect.width() - blurred_edge_thickness;
            auto top_right_inner_height = top_right_corner_rect.height() - blurred_edge_thickness;
            auto bottom_right_inner_width = bottom_right_corner_rect.width() - blurred_edge_thickness;
            auto bottom_right_inner_height = bottom_right_corner_rect.height() - blurred_edge_thickness;
            auto bottom_left_inner_width = bottom_left_corner_rect.width() - blurred_edge_thickness;
            auto bottom_left_inner_height = bottom_left_corner_rect.height() - blurred_edge_thickness;

            Gfx::IntRect top_rect {
                inner_bounding_rect.x() + top_left_inner_width,
                inner_bounding_rect.y(),
                inner_bounding_rect.width() - top_left_inner_width - top_right_inner_width,
                top_left_inner_height
            };
            Gfx::IntRect right_rect {
                inner_bounding_rect.x() + inner_bounding_rect.width() - top_right_inner_width,
                inner_bounding_rect.y() + top_right_inner_height,
                top_right_inner_width,
                inner_bounding_rect.height() - top_right_inner_height - bottom_right_inner_height
            };
            Gfx::IntRect bottom_rect {
                inner_bounding_rect.x() + bottom_left_inner_width,
                inner_bounding_rect.y() + inner_bounding_rect.height() - bottom_right_inner_height,
                inner_bounding_rect.width() - bottom_left_inner_width - bottom_right_inner_width,
                bottom_right_inner_height
            };
            Gfx::IntRect left_rect {
                inner_bounding_rect.x(),
                inner_bounding_rect.y() + top_left_inner_height,
                bottom_left_inner_width,
                inner_bounding_rect.height() - top_left_inner_height - bottom_left_inner_height
            };
            Gfx::IntRect inner = {
                left_rect.x() + left_rect.width(),
                left_rect.y(),
                inner_bounding_rect.width() - left_rect.width() - right_rect.width(),
                inner_bounding_rect.height() - top_rect.height() - bottom_rect.height()
            };

            painter.fill_rect(top_rect, box_shadow_data.color);
            painter.fill_rect(right_rect, box_shadow_data.color);
            painter.fill_rect(bottom_rect, box_shadow_data.color);
            painter.fill_rect(left_rect, box_shadow_data.color);
            painter.fill_rect(inner, box_shadow_data.color);
        };

        auto left_start = inner_bounding_rect.left() - blurred_edge_thickness;
        auto right_start = inner_bounding_rect.left() + inner_bounding_rect.width() + (blurred_edge_thickness - vertical_edge_width);
        auto top_start = inner_bounding_rect.top() - blurred_edge_thickness;
        auto bottom_start = inner_bounding_rect.top() + inner_bounding_rect.height() + (blurred_edge_thickness - horizontal_edge_width);

        // Note: The +1s in a few of the following translations are due to the -1s Gfx::Rect::right() and Gfx::Rect::bottom().
        auto top_left_corner_blit_pos = inner_bounding_rect.top_left().translated(-blurred_edge_thickness, -blurred_edge_thickness);
        auto top_right_corner_blit_pos = inner_bounding_rect.top_right().translated(-top_right_corner_size.width() + 1 + double_radius, -blurred_edge_thickness);
        auto bottom_left_corner_blit_pos = inner_bounding_rect.bottom_left().translated(-blurred_edge_thickness, -bottom_left_corner_size.height() + 1 + double_radius);
        auto bottom_right_corner_blit_pos = inner_bounding_rect.bottom_right().translated(-bottom_right_corner_size.width() + 1 + double_radius, -bottom_right_corner_size.height() + 1 + double_radius);

        auto paint_shadow = [&](Gfx::IntRect clip_rect) {
            Gfx::PainterStateSaver save { painter };
            painter.add_clip_rect(clip_rect);

            paint_shadow_infill();

            // Corners
            painter.blit(top_left_corner_blit_pos, shadow_bitmap, top_left_corner_rect);
            painter.blit(top_right_corner_blit_pos, shadow_bitmap, top_right_corner_rect);
            painter.blit(bottom_left_corner_blit_pos, shadow_bitmap, bottom_left_corner_rect);
            painter.blit(bottom_right_corner_blit_pos, shadow_bitmap, bottom_right_corner_rect);

            // Horizontal edges
            for (auto x = inner_bounding_rect.left() + (bottom_left_corner_size.width() - double_radius); x <= inner_bounding_rect.right() - (bottom_right_corner_size.width() - double_radius); ++x)
                painter.blit({ x, bottom_start }, shadow_bitmap, bottom_edge_rect);
            for (auto x = inner_bounding_rect.left() + (top_left_corner_size.width() - double_radius); x <= inner_bounding_rect.right() - (top_right_corner_size.width() - double_radius); ++x)
                painter.blit({ x, top_start }, shadow_bitmap, top_edge_rect);

            // Vertical edges
            for (auto y = inner_bounding_rect.top() + (top_right_corner_size.height() - double_radius); y <= inner_bounding_rect.bottom() - (bottom_right_corner_size.height() - double_radius); ++y)
                painter.blit({ right_start, y }, shadow_bitmap, right_edge_rect);
            for (auto y = inner_bounding_rect.top() + (top_left_corner_size.height() - double_radius); y <= inner_bounding_rect.bottom() - (bottom_left_corner_size.height() - double_radius); ++y)
                painter.blit({ left_start, y }, shadow_bitmap, left_edge_rect);
        };
}


}
