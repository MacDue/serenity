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

    // auto const& checkbox = static_cast<HTML::HTMLInputElement const&>(layout_box().dom_node());
    if (phase == PaintPhase::Foreground) {
        constexpr float left_offset = 1 / 2.0f;
        constexpr float top_offset = 1 / 2.0f;
        // constexpr float bottom_offset = 1/5.0f;

        auto checkbox_rect = context.enclosing_device_rect(absolute_rect()).to_type<int>();

        auto border_radius = round_to<int>(checkbox_rect.width() * 1 / 5.0f);
        auto line_width = checkbox_rect.width() * 1 / 6.0f;

        // device_pixels_per_css_pixel
        Gfx::AntiAliasingPainter painter { context.painter() };
        // bool enabled = layout_box().dom_node().enabled();
        //  checkbox.checked(), being_pressed()
        painter.fill_rect_with_rounded_corners(checkbox_rect, Gfx::Color::Black, border_radius);
        auto tick_rect = checkbox_rect.to_type<float>().shrunken(line_width * 2, line_width * 2);

        auto rotate_90 = [&](auto point, auto center) {
            using T = decltype(point);
            point -= center;
            return T { -point.y(), point.x() } + center;
        };

        auto shift = AK::Sqrt1_2<float> * line_width / 2;
        Gfx::Line l0 {
            Gfx::FloatPoint(0, tick_rect.height() * top_offset),
            Gfx::FloatPoint(tick_rect.width() * left_offset, tick_rect.height())
        };

        Gfx::Line l1(rotate_90(l0.b(), l0.b()), rotate_90(l0.a(), l0.b()));
        l1 = l1.translated({ -shift, shift });

        // auto direction = (v1-v0)/v0.distance_from(v1);

        // Gfx::Line l1 {
        //     v0.translated(shift, shift),
        //     tick_rect.closest_to(v0 + direction * checkbox_rect.height())
        // };

        painter.draw_line(l0.translated(tick_rect.top_left()), Gfx::Color::White, line_width);
        painter.draw_line(l1.translated(tick_rect.top_left()), Gfx::Color::White, line_width);
    }
}

}
