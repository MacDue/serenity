/*
 * Copyright (c) 2022, Sam Atkins <atkinssj@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Vector.h>
#include <LibGfx/AffineTransform.h>
#include <LibGfx/Color.h>

namespace Web::HTML {

// https://html.spec.whatwg.org/multipage/canvas.html#canvasstate
class CanvasState {
public:
    virtual ~CanvasState() = default;

    void save();
    void restore();
    void reset();
    bool is_context_lost();

    using FillVariant = Variant<Gfx::Color, JS::Handle<CanvasGradient>>;

    struct FillStyle {
        FillStyle(FillVariant fill_style)
            : m_fill_style(fill_style)
        {
        }

        RefPtr<Gfx::FillStyle> to_gfx_fill_style();

        Variant<DeprecatedString, JS::Handle<CanvasGradient>> to_js_fill_style()
        {
            if (m_fill_style.has<JS::Handle<CanvasGradient>>())
                return m_fill_style.get<JS::Handle<CanvasGradient>>();
            return m_fill_style.get<Gfx::Color>().to_deprecated_string();
        }

    private:
        FillVariant m_fill_style;
    };

    // https://html.spec.whatwg.org/multipage/canvas.html#drawing-state
    struct DrawingState {
        Gfx::AffineTransform transform;
        FillStyle fill_style { Gfx::Color::Black };
        FillStyle stroke_style { Gfx::Color::Black };
        float line_width { 1 };
    };
    DrawingState& drawing_state() { return m_drawing_state; }
    DrawingState const& drawing_state() const { return m_drawing_state; }

    void clear_drawing_state_stack() { m_drawing_state_stack.clear(); }
    void reset_drawing_state() { m_drawing_state = {}; }

    virtual void reset_to_default_state() = 0;

protected:
    CanvasState() = default;

private:
    DrawingState m_drawing_state;
    Vector<DrawingState> m_drawing_state_stack;

    // https://html.spec.whatwg.org/multipage/canvas.html#concept-canvas-context-lost
    bool m_context_lost { false };
};

}
