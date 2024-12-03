/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Array.h>
#include <LibGfx/CharacterBitmap.h>
#include <LibGfx/Gradients.h>
#include <LibGfx/LunaWindowTheme.h>
#include <LibGfx/Painter.h>
#include <LibGfx/Palette.h>
#include <LibGfx/StylePainter.h>

namespace Gfx {

// TODO: Somehow allow colors to be configured in the theme .ini file.
static Array const s_luna_title_gradient {
    ColorStop { Color(9, 151, 255), 0.00f },
    ColorStop { Color(0, 83, 238), 0.14f },
    ColorStop { Color(0, 80, 238), 0.40f },
    ColorStop { Color(0, 102, 255), 0.88f },
    ColorStop { Color(0, 102, 255), 0.93f },
    ColorStop { Color(0, 91, 255), 0.95f },
    ColorStop { Color(0, 61, 215), 0.96f },
    ColorStop { Color(0, 61, 215), 1.00f }
};

static Array const s_button_gradient_0 {
    ColorStop { Color(72, 146, 247), 0.0f },
    ColorStop { Color(57, 128, 244), 0.05f },
    ColorStop { Color(57, 128, 244), 1.0f }
};

static Array const s_button_gradient_base {
    ColorStop { Color(72, 146, 247), 0.0f },
    ColorStop { Color(57, 128, 244), 0.05f },
    ColorStop { Color(57, 128, 244), 1.0f }
};

static Array const s_button_gradient_overlay {
    ColorStop { Color(109, 164, 246), 0.0f },
    ColorStop { Color::Transparent, 0.05f },
    ColorStop { Color::Transparent, 1.0f }
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

IntRect LunaWindowTheme::titlebar_rect(WindowType window_type, WindowMode window_mode, IntRect const& window_rect, Palette const& palette) const
{
    auto window_titlebar_height = titlebar_height(window_type, window_mode, palette);
    // FIXME: Theme notifications.
    if (window_type == WindowType::Notification)
        return ClassicWindowTheme::titlebar_rect(window_type, window_mode, window_rect, palette);
    return { 0, 0, window_rect.width() + palette.window_border_thickness() * 2, window_titlebar_height };
}

void LunaWindowTheme::paint_normal_frame(Painter& painter, WindowState window_state, WindowMode window_mode, IntRect const& window_rect, StringView window_title, Bitmap const& icon, Palette const& palette, IntRect const& leftmost_button_rect, int menu_row_count, bool window_modified) const
{
    (void)window_state;
    (void)icon;
    (void)window_modified;
    (void)leftmost_button_rect;

    auto base_color = Color(22, 39, 213);
    auto paint_window_frame = [&](auto rect) {
        int border_thickness = 3;
        auto light_shade = Color(32, 102, 234);
        auto mid_shade = Color(22, 80, 217);

        painter.draw_rect_with_thickness({ rect.x() + border_thickness / 2,
                                             rect.y() + border_thickness / 2,
                                             rect.width() - border_thickness,
                                             rect.height() - border_thickness },
            base_color, border_thickness);

        painter.draw_line(rect.top_left().translated(0, 1), rect.bottom_left(), base_color);
        painter.draw_line(rect.top_left().translated(1, 1), rect.top_right().translated(-1, 1), light_shade);
        painter.draw_line(rect.top_left().translated(1, 1), rect.bottom_left().translated(1, -1), light_shade);
        painter.draw_line(rect.top_left().translated(2, 2), rect.top_right().translated(-2, 2), base_color);
        painter.draw_line(rect.top_left().translated(2, 2), rect.bottom_left().translated(2, -2), base_color);
        painter.draw_line(rect.top_left().translated(3, 3), rect.top_right().translated(-3, 3), base_color);
        painter.draw_line(rect.top_left().translated(3, 3), rect.bottom_left().translated(3, -3), base_color);

        painter.draw_line(rect.top_right(), rect.bottom_right(), base_color);
        painter.draw_line(rect.top_right().translated(-1, 1), rect.bottom_right().translated(-1, -1), mid_shade);
        painter.draw_line(rect.top_right().translated(-2, 2), rect.bottom_right().translated(-2, -2), base_color);
        painter.draw_line(rect.top_right().translated(-3, 3), rect.bottom_right().translated(-3, -3), base_color);
        painter.draw_line(rect.bottom_left(), rect.bottom_right(), base_color);
        painter.draw_line(rect.bottom_left().translated(1, -1), rect.bottom_right().translated(-1, -1), mid_shade);
        painter.draw_line(rect.bottom_left().translated(2, -2), rect.bottom_right().translated(-2, -2), base_color);
        painter.draw_line(rect.bottom_left().translated(3, -3), rect.bottom_right().translated(-3, -3), base_color);
    };

    auto frame_rect = frame_rect_for_window(WindowType::Normal, window_mode, window_rect, palette, menu_row_count);
    frame_rect.set_location({ 0, 0 });
    paint_window_frame(frame_rect);

    auto titlebar_rect = this->titlebar_rect(WindowType::Normal, window_mode, window_rect, palette);
    titlebar_rect.set_height(titlebar_rect.height() + palette.window_border_thickness() + 1);
    painter.fill_rect_with_linear_gradient(titlebar_rect, s_luna_title_gradient, 180);
    auto title_alignment = palette.title_alignment();
    auto& title_font = FontDatabase::window_title_font();
    auto clipped_title_rect = titlebar_rect.translated(7, 0);
    if (!clipped_title_rect.is_empty()) {
        painter.draw_text(clipped_title_rect.translated(1, 2), window_title, title_font, title_alignment, Color(15, 16, 137), Gfx::TextElision::Right);
        // FIXME: The translated(0, 1) wouldn't be necessary if we could center text based on its baseline.
        painter.draw_text(clipped_title_rect.translated(0, 1), window_title, title_font, title_alignment, Color::White, Gfx::TextElision::Right);
    }

    auto loc = titlebar_rect.location();
    auto loc2 = titlebar_rect.location().translated(titlebar_rect.width() - 5, 0);
    painter.draw_rect(titlebar_rect, base_color);

    for (unsigned y = 0; y < s_window_border_radius_mask.height(); y++) {
        for (unsigned x = 0; x < s_window_border_radius_mask.width(); x++) {
            auto bit = s_window_border_radius_mask.bit_at(x, y);
            auto bit2 = s_window_border_radius_accent.bit_at(x, y);
            Gfx::IntRect point(0, 0, 1, 1);
            if (bit) {
                painter.clear_rect(point.translated(loc).translated(x, y), Color());
                painter.clear_rect(point.translated(loc2).translated(5 - x, y), Color());
            }
            if (bit2) {
                painter.clear_rect(point.translated(loc).translated(x, y), base_color);
                painter.clear_rect(point.translated(loc2).translated(5 - x, y), base_color);
            }
        }
    }
}

Vector<IntRect> LunaWindowTheme::layout_buttons(WindowType window_type, WindowMode window_mode, IntRect const& window_rect, Palette const& palette, size_t buttons) const
{
    auto button_rects = ClassicWindowTheme::layout_buttons(window_type, window_mode, window_rect, palette, buttons);
    for (auto& rect : button_rects)
        rect.translate_by(-5, +2);

    return button_rects;
}

void LunaWindowTheme::paint_taskbar(Painter& painter, IntRect const& taskbar_rect, Palette const&) const
{
    painter.fill_rect_with_linear_gradient(taskbar_rect, s_luna_title_gradient, 180);
}

void LunaWindowTheme::paint_button(Painter& painter, IntRect const& rect, Palette const&, ButtonStyle button_style, bool pressed, bool hovered, bool checked, bool enabled, bool focused, bool default_button) const
{
    (void)button_style;
    (void)pressed;
    (void)hovered;
    (void)checked;
    (void)enabled;
    (void)focused;
    (void)default_button;

    if (focused)
        return;

    painter.fill_rect_with_linear_gradient(rect,
        s_button_gradient_base,
        180);
    painter.fill_rect_with_linear_gradient(rect,
        s_button_gradient_overlay,
        160);
    painter.draw_rect(rect, Color(38, 83, 174));
}

}
