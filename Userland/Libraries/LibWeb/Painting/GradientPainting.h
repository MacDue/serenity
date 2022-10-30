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
#include <LibWeb/Painting/PaintContext.h>

namespace Web::Painting {

struct ColorStop {
    Gfx::Color color;
    float position = AK::NaN<float>;
    Optional<float> transition_hint = {};
};

using ColorStopList = Vector<ColorStop, 4>;

struct LinearGradientData {
    float gradient_angle;
    ColorStopList color_stops;
    Optional<float> repeat_length;
};

struct ConicGradientData {
    float start_angle;
    ColorStopList color_stops;
};

LinearGradientData resolve_linear_gradient_data(Layout::Node const&, Gfx::FloatSize const&, CSS::LinearGradientStyleValue const&);
ConicGradientData resolve_conic_gradient_data(Layout::Node const&, CSS::ConicGradientStyleValue const&);

void paint_linear_gradient(PaintContext&, Gfx::IntRect const&, LinearGradientData const&);
void paint_conic_gradient(PaintContext&, Gfx::IntRect const&, ConicGradientData const&);

}
