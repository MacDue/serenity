/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Span.h>
#include <AK/Vector.h>
#include <LibGfx/Color.h>
#include <LibWeb/Forward.h>
#include <LibWeb/CSS/StyleValue.h>
#include <LibWeb/Painting/PaintContext.h>

namespace Web::Painting {

    struct ColorStop {
        Gfx::Color color;
        float position = 0;
    };

    using ColorStopList = Vector<ColorStop, 4>;

    ColorStopList resolve_color_stop_positions(Layout::Node const&, Gfx::FloatRect const &, CSS::LinearGradientStyleValue&);

    void paint_linear_gradient(PaintContext&, Gfx::IntRect const &, float gradient_angle, Span<ColorStop const> color_stop_list);
}
