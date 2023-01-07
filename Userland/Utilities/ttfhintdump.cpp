/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Format.h>
#include <LibCore/ArgsParser.h>
#include <LibGfx/Font/OpenType/Font.h>
#include <LibGfx/Font/OpenType/Hinting/Opcodes.h>
#include <LibMain/Main.h>

using namespace OpenType::Hinting;

static void print_bytes(ReadonlyBytes bytes)
{
    for (auto value : bytes)
        out(", {}", value);
}

static void print_words(ReadonlyBytes bytes)
{
    for (size_t i = 0; i < bytes.size(); i += 2) {
        u16 word = bytes[i] << 8 | bytes[i + 1];
        out(", {}", word);
    }
}

struct InstructionPrinter : InstructionHandler {
    void before_instruction(Context context) override
    {
        auto digits = int(AK::log10(float(context.stream.length()))) + 1;
        out("{:0{}}: ", context.stream.current_position() - 1, digits);
    }

    void print_instruction(Context context)
    {
        outln("{}[]", opcode_name(context.opcode));
    }

    void print_instruction(Context context, bool a)
    {
        outln("{}[{:01b}]", opcode_name(context.opcode), a);
    }

    void print_instruction(Context context, bool a, bool b)
    {
        outln("{}[{:02b}]", opcode_name(context.opcode), (a << 1) | b);
    }

    void print_instruction(Context context, bool a, bool b, bool c, u8 de)
    {
        outln("{}[{:05b}]", opcode_name(context.opcode), (a << 4) | (b << 3) | (c << 2) | de);
    }

    void npush_bytes(Context context, ReadonlyBytes values) override
    {
        out("{}[] {}", opcode_name(context.opcode), values.size());
        print_bytes(values);
        outln();
    }

    void npush_words(Context context, ReadonlyBytes values) override
    {
        out("{}[] {}", opcode_name(context.opcode), values.size() / 2 - 1);
        print_words(values);
        outln();
    }

    void push_bytes(Context context, ReadonlyBytes values) override
    {
        out("{}[{:03b}]", opcode_name(context.opcode), values.size());
        print_bytes(values);
        outln();
    }

    void push_words(Context context, ReadonlyBytes values) override
    {
        out("{}[{:03b}]", opcode_name(context.opcode), values.size() / 2 - 1);
        print_words(values);
        outln();
    }

    void default_handler(Context context) override { print_instruction(context); }
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

ErrorOr<int> serenity_main(Main::Arguments arguments)
{
    Core::ArgsParser args_parser;

    static StringView font_path;
    args_parser.add_positional_argument(font_path, "Path to font", "FILE");
    args_parser.parse(arguments);

    auto font = TRY(OpenType::Font::try_load_from_file(font_path));

    auto program_data = font->font_program_data();
    if (!program_data.has_value() || program_data->size() == 0)
        outln("No font program found");

    InstructionPrinter printer {};
    InstructionStream stream { printer, *program_data };

    while (!stream.at_end())
        stream.process_next_instruction();

    return 0;
}
