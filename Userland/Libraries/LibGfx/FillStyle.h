/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Function.h>
#include <AK/RefCounted.h>
#include <AK/RefPtr.h>
#include <AK/Vector.h>
#include <LibGfx/Color.h>
#include <LibGfx/Gradients.h>
#include <LibGfx/Rect.h>

namespace Gfx {

struct FillStyle : RefCounted<FillStyle> {
    using SamplerFunction = Function<Color(IntPoint)>;
    using FillImplementation = Function<void(SamplerFunction)>;

    virtual void fill(IntRect physical_bounding_box, FillImplementation) = 0;

    virtual ~FillStyle() = default;
};

struct SolidFillStyle : FillStyle {
    static RefPtr<SolidFillStyle> create(Color color)
    {
        return adopt_ref(*new SolidFillStyle(color));
    }

    virtual void fill(IntRect, FillImplementation fill) override
    {
        fill([this](IntPoint) { return m_color; });
    }

private:
    SolidFillStyle(Color color)
        : m_color(color)
    {
    }

    Color m_color;
};

struct GradientFillStyle : FillStyle {
    ErrorOr<void> add_color_stop(float position, Color color, Optional<float> transition_hint = {})
    {
        TRY(add_color_stop(ColorStop { color, position, transition_hint }));
    }

    ErrorOr<void> add_color_stop(ColorStop stop)
    {
        return m_color_stops.try_append(stop);
    }

    void set_repeat_length(float repeat_length)
    {
        m_repeat_length = repeat_length;
    }

    Span<ColorStop const> color_stops() const { return m_color_stops; }
    Optional<float> repeat_length() const { return m_repeat_length; }

private:
    Vector<ColorStop, 4> m_color_stops;
    Optional<float> m_repeat_length;
};

struct LinearGradientFillStyle : GradientFillStyle {
    static RefPtr<LinearGradientFillStyle> create(float angle = 0.0f)
    {
        return adopt_ref(*new LinearGradientFillStyle(angle));
    }

    virtual void fill(IntRect physical_bounding_box, FillImplementation fill);

private:
    LinearGradientFillStyle(float angle)
        : m_angle(angle)
    {
    }

    float m_angle;
};

struct ConicGradientFillStyle : GradientFillStyle {
    static RefPtr<ConicGradientFillStyle> create(IntPoint center, float start_angle = 0.0f)
    {
        return adopt_ref(*new ConicGradientFillStyle(center, start_angle));
    }

    virtual void fill(IntRect physical_bounding_box, FillImplementation fill);

private:
    ConicGradientFillStyle(IntPoint center, float start_angle)
        : m_center(center)
        , m_start_angle(start_angle)
    {
    }

    IntPoint m_center;
    float m_start_angle;
};

struct RadialGradientFillStyle : GradientFillStyle {
    static RefPtr<RadialGradientFillStyle> create(IntPoint center, IntSize size)
    {
        return adopt_ref(*new RadialGradientFillStyle(center, size));
    }

    virtual void fill(IntRect physical_bounding_box, FillImplementation fill);

private:
    RadialGradientFillStyle(IntPoint center, IntSize size)
        : m_center(center)
        , m_size(size)
    {
    }

    IntPoint m_center;
    IntSize m_size;
};

}
