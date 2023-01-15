/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Function.h>
#include <AK/NonnullRefPtr.h>
#include <AK/QuickSort.h>
#include <AK/RefCounted.h>
#include <AK/RefPtr.h>
#include <AK/Vector.h>
#include <LibGfx/Color.h>
#include <LibGfx/Forward.h>
#include <LibGfx/Gradients.h>
#include <LibGfx/Rect.h>

namespace Gfx {

struct FillStyle : RefCounted<FillStyle> {
    virtual ~FillStyle() = default;
    using SamplerFunction = Function<Color(IntPoint)>;
    using FillImplementation = Function<void(SamplerFunction)>;

    friend Painter;
    friend AntiAliasingPainter;

private:
    // Simple fill styles can simply override sample_color() if they can easily generate a color from a coordinate.
    virtual Color sample_color(IntPoint) { return Color(); };

    // Fill styles that have paint time dependent state (e.g. based on the fill size) may find it easier to override fill().
    // If fill() is overridden sample_color() is unused.
    virtual void fill(IntRect physical_bounding_box, FillImplementation fill)
    {
        (void)physical_bounding_box;
        fill([this](IntPoint point) { return sample_color(point); });
    }
};

struct SolidFillStyle : FillStyle {
    static NonnullRefPtr<SolidFillStyle> create(Color color)
    {
        return adopt_ref(*new SolidFillStyle(color));
    }

    virtual Color sample_color(IntPoint) override { return m_color; }

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
        return TRY(add_color_stop(ColorStop { color, position, transition_hint }));
    }

    ErrorOr<void> add_color_stop(ColorStop stop, bool sort = true)
    {
        TRY(m_color_stops.try_append(stop));
        if (sort)
            quick_sort(m_color_stops, [](auto& a, auto& b) { return a.position < b.position; });
        return {};
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
    static NonnullRefPtr<LinearGradientFillStyle> create(float angle = 0.0f)
    {
        return adopt_ref(*new LinearGradientFillStyle(angle));
    }

private:
    virtual void fill(IntRect physical_bounding_box, FillImplementation fill) override;

    LinearGradientFillStyle(float angle)
        : m_angle(angle)
    {
    }

    float m_angle;
};

struct ConicGradientFillStyle : GradientFillStyle {
    static NonnullRefPtr<ConicGradientFillStyle> create(IntPoint center, float start_angle = 0.0f)
    {
        return adopt_ref(*new ConicGradientFillStyle(center, start_angle));
    }

private:
    virtual void fill(IntRect physical_bounding_box, FillImplementation fill) override;

    ConicGradientFillStyle(IntPoint center, float start_angle)
        : m_center(center)
        , m_start_angle(start_angle)
    {
    }

    IntPoint m_center;
    float m_start_angle;
};

struct RadialGradientFillStyle : GradientFillStyle {
    static NonnullRefPtr<RadialGradientFillStyle> create(IntPoint center, IntSize size)
    {
        return adopt_ref(*new RadialGradientFillStyle(center, size));
    }

private:
    virtual void fill(IntRect physical_bounding_box, FillImplementation fill) override;

    RadialGradientFillStyle(IntPoint center, IntSize size)
        : m_center(center)
        , m_size(size)
    {
    }

    IntPoint m_center;
    IntSize m_size;
};

}
