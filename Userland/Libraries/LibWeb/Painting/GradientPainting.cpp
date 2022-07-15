/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Math.h>
#include <LibGfx/Line.h>
#include <LibGfx/Gamma.h>
#include <LibWeb/Painting/GradientPainting.h>

namespace Web::Painting {

static float gradient_angle_radians(float gradient_angle) {
    // Adjust angle so 0 degrees is bottom
    float real_angle = 90 - gradient_angle;
    return real_angle * (AK::Pi<float> / 180);
}

static float calulate_gradient_length(Gfx::IntRect const & gradient_rect, float gradient_angle) {
    float angle = gradient_angle_radians(gradient_angle);
    return abs(gradient_rect.height() * sin(angle)) + abs(gradient_rect.width() * cos(angle));
}

LinearGradientData resolve_linear_gradient_data(Layout::Node const& node, Gfx::FloatRect const & gradient_rect, CSS::LinearGradientStyleValue const & linear_gradient) {
    auto& color_stop_list = linear_gradient.color_stop_list();

    VERIFY(color_stop_list.size() >= 2);
    ColorStopList resolved_color_stops;
    resolved_color_stops.ensure_capacity(color_stop_list.size());
    for (auto& stop: color_stop_list)
        resolved_color_stops.append(ColorStop { .color = stop.color_stop.color });

    auto gradient_angle = linear_gradient.angle(gradient_rect);
    auto gradient_length_px = calulate_gradient_length(gradient_rect.to_rounded<int>(), gradient_angle);
    auto gradient_length = CSS::Length::make_px(gradient_length_px);

    // 1. If the first color stop does not have a position, set its position to 0%.
    auto& first_stop = color_stop_list.first().color_stop;
    resolved_color_stops.first().position = first_stop.length.has_value()
        ? first_stop.length->resolved(node, gradient_length).to_px(node) : 0;
    //    If the last color stop does not have a position, set its position to 100%
    auto& last_stop = color_stop_list.last().color_stop;
    resolved_color_stops.last().position = last_stop.length.has_value()
        ? last_stop.length->resolved(node, gradient_length).to_px(node) : gradient_length_px;

    // 2. If a color stop or transition hint has a position that is less than the
    //    specified position of any color stop or transition hint before it in the list,
    //    set its position to be equal to the largest specified position of any color stop
    //    or transition hint before it.
    auto max_previous_color_stop = resolved_color_stops[0].position;
    for (size_t i = 1; i < color_stop_list.size(); i++) {
        auto& stop = color_stop_list[i];
        if (stop.color_stop.length.has_value()) {
            float value = stop.color_stop.length->resolved(node, gradient_length).to_px(node);
            value = max(value, max_previous_color_stop);
            resolved_color_stops[i].position = value;
            max_previous_color_stop = value;
        }
    }

    // 3. If any color stop still does not have a position, then, for each run of adjacent color stops
    //    without positions, set their positions so that they are evenly spaced between the preceding
    //    and following color stops with positions.
    size_t i = 1;
    auto find_run_end = [&]{
        while (i < color_stop_list.size() - 1 && !color_stop_list[i].color_stop.length.has_value()) {
            i++;
        }
        return i;
    };
    while (i < color_stop_list.size() - 1) {
        auto& stop = color_stop_list[i];
        if (!stop.color_stop.length.has_value()) {
            auto run_start = i - 1;
            auto run_end = find_run_end();
            auto start_position = resolved_color_stops[run_start].position;
            auto end_position = resolved_color_stops[run_end].position;
            auto spacing = (end_position - start_position)/(run_end - run_start + 1);
            for (auto j = run_start + 1; j < run_end; j++) {
                resolved_color_stops[j].position = start_position + (j - run_start) * spacing;
            }
        }
        i++;
    }

    return { gradient_angle, resolved_color_stops };
}

// Note: Gfx::gamma_accurate_blend() is NOT correct for linear gradients!
static Gfx::Color color_mix(Gfx::Color x, Gfx::Color y, float a) {
    auto mix = [&](float x, float y, float a) {
        return x * (1 - a) + y * a;
    };
    return Gfx::Color {
        round_to<u8>(mix(x.red(), y.red(), a)),
        round_to<u8>(mix(x.green(), y.green(), a)),
        round_to<u8>(mix(x.blue(), y.blue(), a)),
        round_to<u8>(mix(x.alpha(), y.alpha(), a)),
    };
}

void paint_linear_gradient(PaintContext& context, Gfx::IntRect const & gradient_rect, LinearGradientData const & data) {
    auto length = calulate_gradient_length(gradient_rect, data.gradient_angle);

    float angle = gradient_angle_radians(data.gradient_angle);
    float sin_angle = sin(angle);
    float cos_angle = cos(angle);
    Gfx::FloatPoint offset { cos_angle * (length / 2), sin_angle * (length / 2)};

    auto center = gradient_rect.center();
    auto start_point = center.to_type<float>() - offset;

    // Rotate gradient line to be horizontal
    auto rotated_start_point_x = start_point.x() * cos_angle - start_point.y() * -sin_angle;

    auto linear_step = [](float min, float max, float value) -> float {
        if (value < min)
            return 0.;
        if (value > max)
            return 1.;
        return (value - min) / (max - min);
    };

    auto& color_stops = data.color_stops;
    for (int y = 0; y < gradient_rect.height(); y++) {
        for (int x = 0; x < gradient_rect.width(); x++) {
            auto x_loc = x * cos_angle - y * -sin_angle;
            Gfx::Color gradient_color = color_mix(
                color_stops[0].color,
                color_stops[1].color,
                linear_step(
                    rotated_start_point_x + color_stops[0].position,
                    rotated_start_point_x + color_stops[1].position,
                    x_loc));
            for (size_t i = 1; i < color_stops.size() - 1; i++) {
                gradient_color = color_mix(
                    gradient_color,
                    color_stops[i + 1].color,
                    linear_step(
                        rotated_start_point_x + color_stops[i].position,
                        rotated_start_point_x + color_stops[i+1].position,
                        x_loc));
            }
            context.painter().set_pixel(gradient_rect.x() + x, gradient_rect.y() + y, gradient_color);
        }
    }
}

}
