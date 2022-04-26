/*
 * Copyright (c) 2021, Ali Mohammad Pur <mpfard@serenityos.org>
 * Copyright (c) 2022, Ben Maxwell <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC optimize("O3")
#endif

#include "FillPathImplementation.h"
#include <AK/Function.h>
#include <LibGfx/AntiAliasingPainter.h>
#include <LibGfx/Path.h>

// Base algorithm from https://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm,
// because there seems to be no other known method for drawing AA'd lines (?)
template<Gfx::AntiAliasingPainter::AntiAliasPolicy policy>
void Gfx::AntiAliasingPainter::draw_anti_aliased_line(FloatPoint const& actual_from, FloatPoint const& actual_to, Color color, float thickness, Gfx::Painter::LineStyle style, Color)
{
    // FIXME: Implement this :P
    VERIFY(style == Painter::LineStyle::Solid);

    auto corrected_thickness = thickness > 1 ? thickness - 1 : thickness;
    auto size = IntSize(corrected_thickness, corrected_thickness);
    auto plot = [&](int x, int y, float c) {
        m_underlying_painter.fill_rect(IntRect::centered_on({ x, y }, size), color.with_alpha(color.alpha() * c));
    };

    auto integer_part = [](float x) { return floorf(x); };
    auto round = [&](float x) { return integer_part(x + 0.5f); };
    auto fractional_part = [&](float x) { return x - floorf(x); };
    auto one_minus_fractional_part = [&](float x) { return 1.0f - fractional_part(x); };

    auto draw_line = [&](float x0, float y0, float x1, float y1) {
        bool steep = fabsf(y1 - y0) > fabsf(x1 - x0);

        if (steep) {
            swap(x0, y0);
            swap(x1, y1);
        }

        if (x0 > x1) {
            swap(x0, x1);
            swap(y0, y1);
        }

        float dx = x1 - x0;
        float dy = y1 - y0;

        float gradient;
        if (dx == 0.0f)
            gradient = 1.0f;
        else
            gradient = dy / dx;

        // Handle first endpoint.
        int x_end = round(x0);
        int y_end = y0 + gradient * (x_end - x0);
        float x_gap = one_minus_fractional_part(x0 + 0.5f);

        int xpxl1 = x_end; // This will be used in the main loop.
        int ypxl1 = integer_part(y_end);

        if (steep) {
            plot(ypxl1, xpxl1, one_minus_fractional_part(y_end) * x_gap);
            plot(ypxl1 + 1, xpxl1, fractional_part(y_end) * x_gap);
        } else {
            plot(xpxl1, ypxl1, one_minus_fractional_part(y_end) * x_gap);
            plot(xpxl1, ypxl1 + 1, fractional_part(y_end) * x_gap);
        }

        float intery = y_end + gradient; // First y-intersection for the main loop.

        // Handle second endpoint.
        x_end = round(x1);
        y_end = y1 + gradient * (x_end - x1);
        x_gap = fractional_part(x1 + 0.5f);
        int xpxl2 = x_end; // This will be used in the main loop
        int ypxl2 = integer_part(y_end);

        if (steep) {
            plot(ypxl2, xpxl2, one_minus_fractional_part(y_end) * x_gap);
            plot(ypxl2 + 1, xpxl2, fractional_part(y_end) * x_gap);
        } else {
            plot(xpxl2, ypxl2, one_minus_fractional_part(y_end) * x_gap);
            plot(xpxl2, ypxl2 + 1, fractional_part(y_end) * x_gap);
        }

        // Main loop.
        if (steep) {
            for (int x = xpxl1 + 1; x <= xpxl2 - 1; ++x) {
                if constexpr (policy == AntiAliasPolicy::OnlyEnds) {
                    plot(integer_part(intery), x, 1);
                } else {
                    plot(integer_part(intery), x, one_minus_fractional_part(intery));
                }
                plot(integer_part(intery) + 1, x, fractional_part(intery));
                intery += gradient;
            }
        } else {
            for (int x = xpxl1 + 1; x <= xpxl2 - 1; ++x) {
                if constexpr (policy == AntiAliasPolicy::OnlyEnds) {
                    plot(x, integer_part(intery), 1);
                } else {
                    plot(x, integer_part(intery), one_minus_fractional_part(intery));
                }
                plot(x, integer_part(intery) + 1, fractional_part(intery));
                intery += gradient;
            }
        }
    };

    auto mapped_from = m_transform.map(actual_from);
    auto mapped_to = m_transform.map(actual_to);
    draw_line(mapped_from.x(), mapped_from.y(), mapped_to.x(), mapped_to.y());
}

void Gfx::AntiAliasingPainter::draw_aliased_line(FloatPoint const& actual_from, FloatPoint const& actual_to, Color color, float thickness, Gfx::Painter::LineStyle style, Color alternate_color)
{
    draw_anti_aliased_line<AntiAliasPolicy::OnlyEnds>(actual_from, actual_to, color, thickness, style, alternate_color);
}

void Gfx::AntiAliasingPainter::draw_line(FloatPoint const& actual_from, FloatPoint const& actual_to, Color color, float thickness, Gfx::Painter::LineStyle style, Color alternate_color)
{
    draw_anti_aliased_line<AntiAliasPolicy::Full>(actual_from, actual_to, color, thickness, style, alternate_color);
}

void Gfx::AntiAliasingPainter::fill_path(Path& path, Color color, Painter::WindingRule rule)
{
    Detail::fill_path<Detail::FillPathMode::AllowFloatingPoints>(*this, path, color, rule);
}

void Gfx::AntiAliasingPainter::stroke_path(Path const& path, Color color, float thickness)
{
    FloatPoint cursor;

    for (auto& segment : path.segments()) {
        switch (segment.type()) {
        case Segment::Type::Invalid:
            VERIFY_NOT_REACHED();
        case Segment::Type::MoveTo:
            cursor = segment.point();
            break;
        case Segment::Type::LineTo:
            draw_line(cursor, segment.point(), color, thickness);
            cursor = segment.point();
            break;
        case Segment::Type::QuadraticBezierCurveTo: {
            auto& through = static_cast<QuadraticBezierCurveSegment const&>(segment).through();
            draw_quadratic_bezier_curve(through, cursor, segment.point(), color, thickness);
            cursor = segment.point();
            break;
        }
        case Segment::Type::CubicBezierCurveTo: {
            auto& curve = static_cast<CubicBezierCurveSegment const&>(segment);
            auto& through_0 = curve.through_0();
            auto& through_1 = curve.through_1();
            draw_cubic_bezier_curve(through_0, through_1, cursor, segment.point(), color, thickness);
            cursor = segment.point();
            break;
        }
        case Segment::Type::EllipticalArcTo:
            auto& arc = static_cast<EllipticalArcSegment const&>(segment);
            draw_elliptical_arc(cursor, segment.point(), arc.center(), arc.radii(), arc.x_axis_rotation(), arc.theta_1(), arc.theta_delta(), color, thickness);
            cursor = segment.point();
            break;
        }
    }
}

void Gfx::AntiAliasingPainter::draw_elliptical_arc(FloatPoint const& p1, FloatPoint const& p2, FloatPoint const& center, FloatPoint const& radii, float x_axis_rotation, float theta_1, float theta_delta, Color color, float thickness, Painter::LineStyle style)
{
    Gfx::Painter::for_each_line_segment_on_elliptical_arc(p1, p2, center, radii, x_axis_rotation, theta_1, theta_delta, [&](FloatPoint const& fp1, FloatPoint const& fp2) {
        draw_line(fp1, fp2, color, thickness, style);
    });
}

void Gfx::AntiAliasingPainter::draw_quadratic_bezier_curve(FloatPoint const& control_point, FloatPoint const& p1, FloatPoint const& p2, Color color, float thickness, Painter::LineStyle style)
{
    Gfx::Painter::for_each_line_segment_on_bezier_curve(control_point, p1, p2, [&](FloatPoint const& fp1, FloatPoint const& fp2) {
        draw_line(fp1, fp2, color, thickness, style);
    });
}

void Gfx::AntiAliasingPainter::draw_cubic_bezier_curve(FloatPoint const& control_point_0, FloatPoint const& control_point_1, FloatPoint const& p1, FloatPoint const& p2, Color color, float thickness, Painter::LineStyle style)
{
    Gfx::Painter::for_each_line_segment_on_cubic_bezier_curve(control_point_0, control_point_1, p1, p2, [&](FloatPoint const& fp1, FloatPoint const& fp2) {
        draw_line(fp1, fp2, color, thickness, style);
    });
}

void Gfx::AntiAliasingPainter::draw_circle(IntPoint center, int radius, Color color)
{
    draw_ellipse_part(center, radius, radius, color, false, {});
}

void Gfx::AntiAliasingPainter::draw_ellipse(IntRect a_rect, Color color)
{
    auto center = a_rect.center();
    int radius_a = a_rect.width() / 2;
    int radius_b = a_rect.height() / 2;
    if (!radius_a || !radius_b)
        return;
    if (radius_a == radius_b) {
        draw_ellipse_part(center, radius_a, radius_a, color, false, {});
        return;
    }
    [[maybe_unused]] auto final_point_1 = draw_ellipse_part(center, radius_a, radius_b, color, false, {});
    draw_ellipse_part(center.translated(radius_a*2*0, 0), radius_b, radius_a, color, true, final_point_1);

    // IntPoint a { final_point_1.i, final_point_1.q};
    // IntPoint b { f2.q, f2.i};

    // int end_dist = a.pixels_moved(b);
    // if (end_dist <= 1) {
        dbgln("Good: {}, {}", radius_a, radius_b);
    // } else {
    //     dbgln("Bad: {}", end_dist);
    // }

    // auto t1 = final_point_1;
    // auto t2 = final_point_2;
    // IntRect fill_rect;
    // if (final_point_1.x() == final_point_2.x()) {
    //     // m_underlying_painter.fill_rect({

    //     // })
    //     final_point_1 -= IntPoint{1, 1};
    //     IntPoint top_corner { -final_point_1.x() - 1, -final_point_1.y() - 1 };
    //     auto p1 = final_point_1 - top_corner;
    //     fill_rect = IntRect{
    //         top_corner.x(),
    //         top_corner.y(),
    //         p1.x() + 1, p1.y() +1
    //     };
    // } else if (final_point_1.y() == final_point_2.y()) {
    //     final_point_2 -= IntPoint{1, 1};
    //     IntPoint top_corner { -final_point_2.x() - 1, -final_point_2.y() - 1 };
    //     auto p1 = final_point_2 - top_corner;
    //     fill_rect = IntRect{
    //         top_corner.x(),
    //         top_corner.y(),
    //         p1.x() + 1, p1.y() + 1
    //     };
    // } else {
    //     if (final_point_1.x() < final_point_2.x()) {
    //     final_point_1 -= IntPoint{0, 1};
    //     IntPoint top_corner { -final_point_1.x() - 1, -final_point_1.y() - 1 };
    //     auto p1 = final_point_1 - top_corner;
    //     fill_rect = IntRect{
    //         top_corner.x(),
    //         top_corner.y(),
    //         p1.x() + 1, p1.y() +1
    //     };
    //     } else {
    //                 final_point_2 -= IntPoint{1, 0};
    //     IntPoint top_corner { -final_point_2.x() - 1, -final_point_2.y() - 1 };
    //     auto p1 = final_point_2 - top_corner;
    //     fill_rect = IntRect{
    //         top_corner.x(),
    //         top_corner.y(),
    //         p1.x() + 1, p1.y() +1
    //     };
    //     }
    //     // TODO case:
    //         // Cyan
    //         //     Red
    // }
    // m_underlying_painter.fill_rect(fill_rect.translated(center), color);
    // m_underlying_painter.set_pixel(t1 + center, Color::NamedColor::Red);
    // m_underlying_painter.set_pixel(t2 + center, Color::NamedColor::Cyan);
//    m_underlying_painter.set_pixel(final_point_1 + center, Color::NamedColor::DarkCyan);
//     m_underlying_painter.set_pixel(final_point_2 + center, Color::NamedColor::DarkRed);

}

Gfx::AntiAliasingPainter::FillRange Gfx::AntiAliasingPainter::draw_ellipse_part(
    IntPoint center, int radius_a, int radius_b, Color color, bool flip_x_and_y, Optional<Gfx::AntiAliasingPainter::FillRange> fill_range)
{
    /*
      Algorithm from: https://cs.uwaterloo.ca/research/tr/1984/CS-84-38.pdf
    */

    center *= m_underlying_painter.scale();
    radius_a *= m_underlying_painter.scale();
    radius_b *= m_underlying_painter.scale();

    // If this is a ellipse everything can be drawn in one pass with 8 way symmetry
    const bool is_circle = radius_a == radius_b;

    // These happen to be the same here, but are treated separately in the paper:
    // intensity is the fill alpha
    int const intensity = 255;
    // 0 to subpixel_resolution is the range of alpha values for the circle edges
    int const subpixel_resolution = intensity;

    // Current pixel address
    int i = 0;
    int q = radius_b;

    // 1st and 2nd order differences of y
    int delta_y = 0;
    int delta2_y = 0;

    const int a_squared = radius_a * radius_a;
    const int b_squared = radius_b * radius_b;

    // Exact and predicted values of f(i) -- the ellipse equation scaled by subpixel_resolution
    int y = subpixel_resolution * radius_b;
    int y_hat = 0;

    // The value of f(i)*f(i)
    int f_squared = y * y;

    // 1st and 2nd order differences of f(i)*f(i)
    int delta_f_squared = -(static_cast<int64_t>(b_squared) * subpixel_resolution * subpixel_resolution) / a_squared;
    int delta2_f_squared = 2 * delta_f_squared;

    // edge_intersection_area/subpixel_resolution = percentage of pixel intersected by circle
    // (aka the alpha for the pixel)
    int edge_intersection_area = 0;
    int old_area = edge_intersection_area;

    auto predict = [&] {
        delta_y += delta2_y;
        // y_hat is the predicted value of f(i)
        y_hat = y + delta_y;
    };

    auto minimize = [&] {
        // Initialize the minimization
        delta_f_squared += delta2_f_squared;
        f_squared += delta_f_squared;

        int min_squared_error = y_hat * y_hat - f_squared;
        int prediction_overshot = 1;
        y = y_hat;

        // Force error negative
        if (min_squared_error > 0) {
            min_squared_error = -min_squared_error;
            prediction_overshot = -1;
        }

        // Minimize
        int previous_error = min_squared_error;
        while (min_squared_error < 0) {
            y += prediction_overshot;
            previous_error = min_squared_error;
            min_squared_error += y + y - prediction_overshot;
        }

        if (min_squared_error + previous_error > 0)
            y -= prediction_overshot;
    };

    auto correct = [&] {
        int error = y - y_hat;

        if (!is_circle) {
            // FIXME: The alpha values for the edges are too low, reducing the error
            // here works has a quick fix, but is probably not the right place.
            // (This issue seems to exist in the base algorithm)
            error /= 4;
        }

        delta2_y += error;
        delta_y += error;
    };

    auto pixel = [&](int x, int y, int alpha) {
        if (alpha <= 0 || alpha > 255)
            return;
        if (flip_x_and_y)
            swap(x, y);
        auto pixel_colour = color;
        // dbgln("{}", (alpha*255)/color.alpha());
        pixel_colour.set_alpha((alpha * color.alpha())/255);
        m_underlying_painter.set_pixel(center + IntPoint { x, y }, pixel_colour, true);
    };

    int min_fill_x = 0x7FFFFFFF;
    int max_fill_x = 0;
    auto fill = [&](int x, int ymax, int ymin, int alpha) {
        min_fill_x = min(x, min_fill_x);
        max_fill_x = max(x, max_fill_x);
        auto o = color;
        color = flip_x_and_y ? Color::NamedColor::Red : Color::NamedColor::Cyan;
        color.set_alpha(o.alpha());
        while (ymin <= ymax) {
            if (flip_x_and_y && fill_range.has_value()) {
                if (ymin >= fill_range->min_x && ymin <= fill_range->max_x) {
                    ymin = fill_range->max_x + 1;
                    if (ymin > ymax) break;
                }
            }
            pixel(x, ymin, alpha);
            ymin += 1;
        }
        color = o;
    };

    auto symmetric_pixel = [&](int x, int y, int alpha) {
        pixel(x, y, alpha);
        pixel(x, -y - 1, alpha);
        pixel(-x - 1, -y - 1, alpha);
        pixel(-x - 1, y, alpha);
        if (is_circle) {
            pixel(y, x, alpha);
            pixel(y, -x - 1, alpha);
            pixel(-y - 1, -x - 1, alpha);
            pixel(-y - 1, x, alpha);
        }
    };

    // These are calculated incrementally (as it is possibly a tiny bit faster)
    int ib_squared = 0;
    int qa_squared = q * a_squared;

    auto in_symmetric_region = [&] {
        // Main fix two stop cond here
        return is_circle ? i < q : ib_squared < qa_squared;
    };

    // Draws a 8 octants for a circle or 4 quadrants for a (partial) elipse
    while (in_symmetric_region()) {
        predict();
        minimize();
        correct();
        old_area = edge_intersection_area;
        edge_intersection_area += delta_y;
        if (edge_intersection_area >= 0) {
            // Single pixel on perimeter
            symmetric_pixel(i, q, (edge_intersection_area + old_area) / 2);
            fill(i, q - 1, -q, intensity);
            fill(-i - 1, q - 1, -q, intensity);
        } else {
            // Two pixels on perimeter
            edge_intersection_area += subpixel_resolution;
            symmetric_pixel(i, q, old_area / 2);
            q -= 1;
            qa_squared -= a_squared;
            fill(i, q - 1, -q, intensity);
            fill(-i - 1, q - 1, -q, intensity);
            if (in_symmetric_region()) {
                symmetric_pixel(i, q, (edge_intersection_area + subpixel_resolution) / 2);
                if (is_circle) {
                    fill(q, i - 1, -i, intensity);
                    fill(-q - 1, i - 1, -i, intensity);
                }
            } else {
                edge_intersection_area += subpixel_resolution;
            }
        }
        i += 1;
        ib_squared += b_squared;
    }

    if (is_circle) {
        int alpha = edge_intersection_area / 2;
        pixel(q, q, alpha);
        pixel(-q - 1, q, alpha);
        pixel(-q - 1, -q - 1, alpha);
        pixel(q, -q - 1, alpha);
    }

    dbgln("{}, {}", i, q);
    return FillRange { min_fill_x, max_fill_x, i, q };
}

void Gfx::AntiAliasingPainter::fill_rect_with_rounded_corners(IntRect const& a_rect, Color color, int radius)
{
    fill_rect_with_rounded_corners(a_rect, color, radius, radius, radius, radius);
}

void Gfx::AntiAliasingPainter::fill_rect_with_rounded_corners(IntRect const& a_rect, Color color, int top_left_radius, int top_right_radius, int bottom_right_radius, int bottom_left_radius)
{
    if (!top_left_radius && !top_right_radius && !bottom_right_radius && !bottom_left_radius)
        return m_underlying_painter.fill_rect(a_rect, color);

    if (color.alpha() == 0)
        return;

    IntPoint top_left_corner {
        a_rect.x() + top_left_radius,
        a_rect.y() + top_left_radius,
    };
    IntPoint top_right_corner {
        a_rect.x() + a_rect.width() - top_right_radius,
        a_rect.y() + top_right_radius,
    };
    IntPoint bottom_left_corner {
        a_rect.x() + bottom_left_radius,
        a_rect.y() + a_rect.height() - bottom_right_radius
    };
    IntPoint bottom_right_corner {
        a_rect.x() + a_rect.width() - bottom_left_radius,
        a_rect.y() + a_rect.height() - bottom_left_radius
    };

    IntRect top_rect {
        a_rect.x() + top_left_radius,
        a_rect.y(),
        a_rect.width() - top_left_radius - top_right_radius,
        top_left_radius
    };
    IntRect right_rect {
        a_rect.x() + a_rect.width() - top_right_radius,
        a_rect.y() + top_right_radius,
        top_right_radius,
        a_rect.height() - top_right_radius - bottom_right_radius
    };
    IntRect bottom_rect {
        a_rect.x() + bottom_left_radius,
        a_rect.y() + a_rect.height() - bottom_right_radius,
        a_rect.width() - bottom_left_radius - bottom_right_radius,
        bottom_right_radius
    };
    IntRect left_rect {
        a_rect.x(),
        a_rect.y() + top_left_radius,
        bottom_left_radius,
        a_rect.height() - top_left_radius - bottom_left_radius
    };

    IntRect inner = {
        left_rect.x() + left_rect.width(),
        left_rect.y(),
        a_rect.width() - left_rect.width() - right_rect.width(),
        a_rect.height() - top_rect.height() - bottom_rect.height()
    };

    m_underlying_painter.fill_rect(top_rect, color);
    m_underlying_painter.fill_rect(right_rect, color);
    m_underlying_painter.fill_rect(bottom_rect, color);
    m_underlying_painter.fill_rect(left_rect, color);
    m_underlying_painter.fill_rect(inner, color);

    // FIXME: Don't draw a whole circle each time
    if (top_left_radius)
        draw_circle(top_left_corner, top_left_radius, color);
    if (top_right_radius)
        draw_circle(top_right_corner, top_right_radius, color);
    if (bottom_left_radius)
        draw_circle(bottom_left_corner, bottom_left_radius, color);
    if (bottom_right_radius)
        draw_circle(bottom_right_corner, bottom_right_radius, color);
}
