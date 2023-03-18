/*
 * Copyright (c) 2018-2022, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGUI/Event.h>
#include <LibGfx/AntiAliasingPainter.h>
#include <LibWeb/HTML/BrowsingContext.h>
#include <LibWeb/HTML/HTMLImageElement.h>
#include <LibWeb/Layout/CheckBox.h>
#include <LibWeb/Layout/Label.h>
#include <LibWeb/Painting/CheckBoxPaintable.h>
namespace Web::Painting {

JS::NonnullGCPtr<CheckBoxPaintable> CheckBoxPaintable::create(Layout::CheckBox const& layout_box)
{
    return layout_box.heap().allocate_without_realm<CheckBoxPaintable>(layout_box);
}

CheckBoxPaintable::CheckBoxPaintable(Layout::CheckBox const& layout_box)
    : LabelablePaintable(layout_box)
{
}

Layout::CheckBox const& CheckBoxPaintable::layout_box() const
{
    return static_cast<Layout::CheckBox const&>(layout_node());
}

Layout::CheckBox& CheckBoxPaintable::layout_box()
{
    return static_cast<Layout::CheckBox&>(layout_node());
}

void CheckBoxPaintable::paint(PaintContext& context, PaintPhase phase) const
{
    if (!is_visible())
        return;

    PaintableBox::paint(context, phase);

    // Make a CrachertBitmap -> Bitmap mathod
    // -> some coloring
    // -> use accent_color
    // auto const& checkbox = static_cast<HTML::HTMLInputElement const&>(layout_box().dom_node());
    if (phase == PaintPhase::Foreground) {
        constexpr float tick_width = 0.5f;
        constexpr float tick_height = 0.7f;
        constexpr float offset_y = -0.07f;
        constexpr float angle = AK::Pi<float> / 4;

        auto checkbox_rect = context.enclosing_device_rect(absolute_rect()).to_type<int>().to_type<float>();

        auto border_radius = round_to<int>(checkbox_rect.width() * 1 / 5.0f);
        auto line_width = round_to<int>(checkbox_rect.width() * 1 / 6.0f) | 1;

        Gfx::AntiAliasingPainter painter { context.painter() };
        // bool enabled = layout_box().dom_node().enabled();
        //  checkbox.checked(), being_pressed()
        painter.fill_rect_with_rounded_corners(checkbox_rect.to_type<int>(), Gfx::Color::Black, border_radius);

        auto tick_rect = Gfx::FloatRect { {}, checkbox_rect.size().scaled_by(tick_width, tick_height) }.centered_within(checkbox_rect);

        auto center = tick_rect.center();
        auto position_line = [=](auto line) {
            return line.translated(-center).rotated(angle).translated(center).translated({ 0, checkbox_rect.height() * offset_y });
        };
        auto shift = -line_width / 2;
        painter.draw_line(position_line(Gfx::Line {
                              tick_rect.bottom_left().translated(0, shift),
                              tick_rect.bottom_right().translated(0, shift) }),
            Gfx::Color::White, line_width);
        painter.draw_line(position_line(Gfx::Line {
                              tick_rect.bottom_right().translated(shift, 0),
                              tick_rect.top_right().translated(shift, 0),
                          }),
            Gfx::Color::White, line_width);
    }
}

}
