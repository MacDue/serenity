/*
 * Copyright (c) 2021-2022, Linus Groh <linusg@serenityos.org>
 * Copyright (c) 2022, Sam Atkins <atkinssj@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/HTML/Canvas/CanvasState.h>

namespace Web::HTML {

// https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-save
void CanvasState::save()
{
    // The save() method steps are to push a copy of the current drawing state onto the drawing state stack.
    m_drawing_state_stack.append(m_drawing_state);
}

// https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-restore
void CanvasState::restore()
{
    // The restore() method steps are to pop the top entry in the drawing state stack, and reset the drawing state it describes. If there is no saved state, then the method must do nothing.
    if (m_drawing_state_stack.is_empty())
        return;
    m_drawing_state = m_drawing_state_stack.take_last();
}

// https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-reset
void CanvasState::reset()
{
    // The reset() method steps are to reset the rendering context to its default state.
    reset_to_default_state();
}

// https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-iscontextlost
bool CanvasState::is_context_lost()
{
    // The isContextLost() method steps are to return this's context lost.
    return m_context_lost;
}

NonnullRefPtr<Gfx::FillStyle> CanvasState::FillStyle::to_gfx_fill_style()
{
    return m_color_fill_style.visit(
        [&](Gfx::Color color) {
            if (m_color_fill_style)
                return m_color_fill_style;
            m_color_fill_style = Gfx::SolidFillStyle::create(color);
            return m_color_fill_style;
        },
        [&](auto& gradient) {
            return gradient->to_gfx_fill_style();
        });
}

Gfx::Color CanvasState::FillStyle::to_color_fill_but_fixme_should_accept_any_fill_style() const
{
    return as_color().value_or(Gfx::Color::Black);
}

Optional<Gfx::Color> CanvasState::FillStyle::as_color() const
{
    if (auto* color = m_fill_style.get_pointer<Gfx::Color>())
        return *color;
    return {};
}

}
