/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Array.h>
#include <LibGfx/AreoWindowTheme.h>
#include <LibGfx/CharacterBitmap.h>
#include <LibGfx/Gradients.h>
#include <LibGfx/Painter.h>
#include <LibGfx/Palette.h>
#include <LibGfx/StylePainter.h>

namespace Gfx {

static const Array s_areo_title_gradient {
    ColorStop { Color(25, 40, 55, 191), 0.35f },
    ColorStop { Color(65, 85, 100, 191), 0.40f },
    ColorStop { Color(65, 85, 100, 191), 0.42f },
    ColorStop { Color(25, 40, 55, 191), 0.50f },
    ColorStop { Color(25, 40, 55, 191), 0.55f },
    ColorStop { Color(70, 85, 100, 191), 0.60f },
    ColorStop { Color(70, 85, 100, 191), 0.75f },
    ColorStop { Color(25, 40, 55, 191), 0.90f }
};

static constexpr Gfx::CharacterBitmap s_window_border_radius_mask {
    "#####"
    "###  "
    "##   "
    "#    "
    "#    "sv,
    5, 5
};

static constexpr Gfx::CharacterBitmap s_window_border_radius_accent {
    "     "
    "   ##"
    "  #  "
    " #   "
    " #   "sv,
    5, 5
};

static constexpr Gfx::CharacterBitmap s_window_border_radius_accent2 {
    "     "
    "     "
    "   ##"
    "  #  "
    "  #  "sv,
    5, 5
};

IntRect AreoWindowTheme::titlebar_rect(WindowType window_type, WindowMode window_mode, IntRect const& window_rect, Palette const& palette) const
{
    auto window_titlebar_height = titlebar_height(window_type, window_mode, palette);
    // TODO:
    if (window_type == WindowType::Notification)
        return {};
    return { 0, 0, window_rect.width() + palette.window_border_thickness() * 2, window_titlebar_height };
}

void AreoWindowTheme::paint_normal_frame(Painter& painter, WindowState window_state, WindowMode window_mode, IntRect const& window_rect, StringView window_title, Bitmap const& icon, Palette const& palette, IntRect const& leftmost_button_rect, int menu_row_count, bool window_modified) const
{
    (void)window_state;
    (void)icon;
    (void)window_modified;

    (void)s_window_border_radius_mask;

    auto base_color = Color(2, 3, 4, 219);

    (void)leftmost_button_rect;

    auto frame_rect = frame_rect_for_window(WindowType::Normal, window_mode, window_rect, palette, menu_row_count);
    frame_rect.set_location({ 0, 0 });
    frame_rect.shrink(0, 1, 1, 1);
    // paint_window_frame(frame_rect);

    auto& title_font = FontDatabase::window_title_font();

    painter.fill_rect(frame_rect, Color(235, 235, 236, 150));

    painter.fill_rect_with_linear_gradient(frame_rect, s_areo_title_gradient, 45, 0.9f);

    auto title_alignment = palette.title_alignment();
    auto titlebar_rect = this->titlebar_rect(WindowType::Normal, window_mode, window_rect, palette);
    titlebar_rect.set_height(titlebar_rect.height() + palette.window_border_thickness() + 1);

    auto clipped_title_rect = titlebar_rect.translated(7, 0);
    if (!clipped_title_rect.is_empty()) {
        painter.draw_text(clipped_title_rect.translated(1, 2), window_title, title_font, title_alignment, Color(15, 16, 137), Gfx::TextElision::Right);
        // FIXME: The translated(0, 1) wouldn't be necessary if we could center text based on its baseline.
        painter.draw_text(clipped_title_rect.translated(0, 1), window_title, title_font, title_alignment, Color::White, Gfx::TextElision::Right);
    }

    auto loc = frame_rect.location();
    auto loc2 = frame_rect.location().translated(frame_rect.width() - 5, 0);
    painter.draw_rect_with_thickness(frame_rect, base_color, 1);
    painter.draw_rect_with_thickness(frame_rect.shrunken(1, 1, 1, 1), Color(235, 235, 236, 170), 1);
    auto inner = frame_rect.shrunken(26, 5, 5, 5);
    painter.draw_rect_with_thickness(inner.inflated(1, 1, 1, 1), Color(235, 235, 236, 110), 1);
    painter.draw_rect_with_thickness(inner, base_color.with_alpha(110), 1);

    for (unsigned y = 0; y < s_window_border_radius_mask.height(); y++) {
        for (unsigned x = 0; x < s_window_border_radius_mask.width(); x++) {
            auto bit = s_window_border_radius_mask.bit_at(x, y);
            auto bit2 = s_window_border_radius_accent.bit_at(x, y);
            auto bit3 = s_window_border_radius_accent2.bit_at(x, y);
            if (bit) {
                painter.set_pixel(loc.translated(x, y), Color());
                painter.set_pixel(loc2.translated(5 - x, y), Color());
            }
            if (bit2) {
                painter.set_pixel(loc.translated(x, y), base_color, true);
                painter.set_pixel(loc2.translated(5 - x, y), base_color, true);
            }
            if (bit3) {
                painter.set_pixel(loc.translated(x, y), Color(235, 235, 236, 170), true);
                painter.set_pixel(loc2.translated(5 - x, y), Color(235, 235, 236, 170), true);
            }
        }
    }
}

Vector<IntRect> AreoWindowTheme::layout_buttons(WindowType window_type, WindowMode window_mode, IntRect const& window_rect, Palette const& palette, size_t buttons) const
{
    auto button_rects = ClassicWindowTheme::layout_buttons(window_type, window_mode, window_rect, palette, buttons);
    for (auto& rect : button_rects) {
        rect.translate_by(-6, 1);
    }
    return button_rects;
}

void AreoWindowTheme::paint_taskbar(Painter& painter, IntRect const& taskbar_rect, Palette const&) const
{
    painter.fill_rect_with_linear_gradient(taskbar_rect, s_areo_title_gradient, 45, 0.9f);
}

}
