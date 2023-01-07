/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibGfx/Font/OpenType/Hinting/Opcodes.h>

namespace OpenType::Hinting {

struct InstructionPrinter : InstructionHandler {

    void print_instruction(Context context);
    void print_instruction(Context context, bool a);
    void print_instruction(Context context, bool a, bool b);
    void print_instruction(Context context, bool a, bool b, bool c, u8 de);

    void default_handler(Context context) override { print_instruction(context); }

    void npush_bytes(Context, ReadonlyBytes) override;
    void npush_words(Context, ReadonlyBytes) override;
    void push_bytes(Context, ReadonlyBytes) override;
    void push_words(Context, ReadonlyBytes) override;

    void set_freedom_and_projection_vectors_to_coordinate_axis(Context context, bool a) override { print_instruction(context, a); }
    void set_projection_vector_to_coordinate_axis(Context context, bool a) override { print_instruction(context, a); }
    void set_freedom_vector_to_coordinate_axis(Context context, bool a) override { print_instruction(context, a); }
    void set_projection_vector_to_line(Context context, bool a) override { print_instruction(context, a); }
    void set_freedom_vector_vector_to_line(Context context, bool a) override { print_instruction(context, a); }
    void set_dual_projection_vector_to_line(Context context, bool a) override { print_instruction(context, a); }
    void measure_distance(Context context, bool a) override { print_instruction(context, a); }

    void shift_point_by_last_point(Context context, bool a) override { print_instruction(context, a); }
    void shift_contour_by_last_point(Context context, bool a) override { print_instruction(context, a); }
    void shift_zone_by_last_point(Context context, bool a) override { print_instruction(context, a); }
    void move_stack_indirect_relative_point(Context context, bool a) override { print_instruction(context, a); }
    void move_direct_absolute_point(Context context, bool a) override { print_instruction(context, a); }
    void move_indirect_absolute_point(Context context, bool a) override { print_instruction(context, a); }
    void move_direct_relative_point(Context context, bool a, bool b, bool c, u8 de) override { print_instruction(context, a, b, c, de); }
    void move_indirect_relative_point(Context context, bool a, bool b, bool c, u8 de) override { print_instruction(context, a, b, c, de); }
    void interpolate_untouched_points_through_outline(Context context, bool a) override { print_instruction(context, a); };

    void round(Context context, bool a, bool b) override { print_instruction(context, a, b); }
    void no_round(Context context, bool a, bool b) override { print_instruction(context, a, b); }
};

}
