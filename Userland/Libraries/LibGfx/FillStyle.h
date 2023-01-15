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
    void add_color_stop(float position, Color color, Optional<float> transition_hint = {})
    {
        add_color_stop(ColorStop { color, position, transition_hint });
    }

    void add_color_stop(ColorStop stop, bool sort = true)
    {
        m_color_stops.append(stop);
        if (sort)
            quick_sort(m_color_stops, [](auto& a, auto& b) { return a.position < b.position; });
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

// These fill styles are based on the CSS gradients. They are relative to the filled
// shape and support premultiplied alpha.

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

// The following fill styles implement the gradients required for the HTML canvas.
// This gradients are (unlike CSS) not relative to the filled shape, and do not
// support premultiplied alpha.

struct CanvasLinearGradientFillStyle : GradientFillStyle {
    static NonnullRefPtr<CanvasLinearGradientFillStyle> create(FloatPoint p0, FloatPoint p1)
    {
        return adopt_ref(*new CanvasLinearGradientFillStyle(p0, p1));
    }

private:
    virtual void fill(IntRect physical_bounding_box, FillImplementation fill) override;

    CanvasLinearGradientFillStyle(FloatPoint p0, FloatPoint p1)
        : m_p0(p0)
        , m_p1(p1)
    {
    }

    FloatPoint m_p0;
    FloatPoint m_p1;
};

struct CanvasConicGradientFillStyle : GradientFillStyle {
    static NonnullRefPtr<CanvasConicGradientFillStyle> create(FloatPoint center, float start_angle = 0.0f)
    {
        return adopt_ref(*new CanvasConicGradientFillStyle(center, start_angle));
    }

private:
    virtual void fill(IntRect physical_bounding_box, FillImplementation fill) override;

    CanvasConicGradientFillStyle(FloatPoint center, float start_angle)
        : m_center(center)
        , m_start_angle(start_angle)
    {
    }

    FloatPoint m_center;
    float m_start_angle;
};

struct CanvasRadialGradientFillStyle : GradientFillStyle {
    static NonnullRefPtr<CanvasRadialGradientFillStyle> create(FloatPoint start_center, float start_radius, FloatPoint end_center, float end_radius)
    {
        return adopt_ref(*new CanvasRadialGradientFillStyle(start_center, start_radius, end_center, end_radius));
    }

private:
    virtual void fill(IntRect physical_bounding_box, FillImplementation fill) override;

    CanvasRadialGradientFillStyle(FloatPoint start_center, float start_radius, FloatPoint end_center, float end_radius)
        : m_start_center(start_center)
        , m_start_radius(start_radius)
        , m_end_center(end_center)
        , m_end_radius(end_radius)
    {
    }

    FloatPoint m_start_center;
    float m_start_radius;
    FloatPoint m_end_center;
    float m_end_radius;
};

}
