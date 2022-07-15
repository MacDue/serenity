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

ColorStopList resolve_color_stop_positions(Layout::Node const& node, Gfx::FloatRect const & gradient_rect, CSS::LinearGradientStyleValue& linear_gradient) {
    auto& color_stop_list = linear_gradient.color_stop_list();

    VERIFY(color_stop_list.size() >= 2);
    ColorStopList resolved_color_stops;
    resolved_color_stops.ensure_capacity(color_stop_list.size());
    for (auto& stop: color_stop_list)
        resolved_color_stops.append(ColorStop { .color = stop.color_stop.color });

    auto gradient_length_px = calulate_gradient_length(gradient_rect.to_rounded<int>(), linear_gradient.angle(gradient_rect));
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
    }

    return resolved_color_stops;
}

void paint_linear_gradient(PaintContext& context, Gfx::IntRect const & gradient_rect, float gradient_angle, Span<ColorStop const> color_stop_list) {
    auto length = calulate_gradient_length(gradient_rect, gradient_angle);

    float angle = gradient_angle_radians(gradient_angle);
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

    for (int y = 0; y < gradient_rect.height(); y++) {
        for (int x = 0; x < gradient_rect.width(); x++) {
            auto x_loc = x * cos_angle - y * -sin_angle;
            Gfx::Color gradient_color = Gfx::gamma_accurate_blend(
                color_stop_list[0].color,
                color_stop_list[1].color,
                linear_step(
                    rotated_start_point_x + color_stop_list[0].position,
                    rotated_start_point_x + color_stop_list[1].position,
                    x_loc));
            for (size_t i = 1; i < color_stop_list.size() - 1; i++) {
                gradient_color = Gfx::gamma_accurate_blend(
                    gradient_color,
                    color_stop_list[1].color,
                    linear_step(
                        rotated_start_point_x + color_stop_list[i].position,
                        rotated_start_point_x + color_stop_list[i+1].position,
                        x_loc));
            }
            context.painter().set_pixel(gradient_rect.x() + x, gradient_rect.y() + y, gradient_color);
        }
    }
}

/*
float gradientLineLen(float angle, vec2 size) {
    return abs(size.x * sin(angle)) + abs(size.y * cos(angle));
}

float linearStep(float min, float max, float val) {
   if (val < min) return 0.;
   if (val > max) return 1.;
   float c = (val - min)/(max - min);
   return c;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float cssAngle = 90.;
    float angle = 90. - cssAngle;
    float angle_rads = angle*(3.14/180.);

    vec2 center = iResolution.xy/2.;
    float grad_len = gradientLineLen(angle_rads, iResolution.yx);
    float l = grad_len / 2.;
    vec2 offset = vec2(cos(angle_rads) * l, sin(angle_rads) * l);

    // these should be uniforms
    vec2 gradient_start_pos = center - offset; // top-left
    vec2 gradient_end_pos = center + offset; // bottom-right

    // define colors and stops
    const int num_stops = 4;
    float stops[32];
    vec4 colors[32];
    stops[0] = 0.0;
    stops[1] = 1./3.;
    stops[2] = 2./3.;
    stops[3] = 1.0;
    colors[0] = vec4(1.0, 0.0, 0.0, 1.0);
    colors[1] = vec4(0.0, 0.0, 0.0, 1.0);
    colors[2] = vec4(1.0, 1.0, 0.0, 1.0);
    colors[3] = vec4(0.0, 1.0, 1.0, 1.0);


	vec2 uv = fragCoord.xy;

    float alpha = angle_rads;

    float gradient_startpos_rotated_x = gradient_start_pos.x * cos(-alpha) - gradient_start_pos.y * sin(-alpha);
    float gradient_endpos_rotated_x = gradient_end_pos.x * cos(-alpha) - gradient_end_pos.y * sin(-alpha);
    float len = gradient_endpos_rotated_x - gradient_startpos_rotated_x;
    float x_loc_rotated = uv.x * cos(-alpha) - uv.y * sin(-alpha);


    if (num_stops == 1) {
        fragColor = colors[0];
    } else if (num_stops > 1) {
        fragColor = mix(colors[0], colors[1], linearStep(
            gradient_startpos_rotated_x + stops[0] * len,
            gradient_startpos_rotated_x + stops[1] * len,
            x_loc_rotated
        ));
        for (int i = 1; i < 32 - 1; i++) {
            if (i < num_stops - 1) {
                fragColor = mix(fragColor, colors[i + 1], linearStep(
                    gradient_startpos_rotated_x + stops[i] * len,
                    gradient_startpos_rotated_x + stops[i + 1] * len,
                    x_loc_rotated
                ));
            } else { break; }
        }
    }
}

*/

}
