/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Format.h>
#include <AK/Utf8View.h>
#include <LibCore/ArgsParser.h>
#include <LibGfx/Font/OpenType/Font.h>
#include <LibGfx/Font/OpenType/Hinting/Opcodes.h>
#include <LibMain/Main.h>

using namespace OpenType::Hinting;

static void print_bytes(ReadonlyBytes bytes)
{
    for (auto value : bytes)
        out(", \e[92m{}\e[0m", value);
}

static void print_words(ReadonlyBytes bytes)
{
    for (size_t i = 0; i < bytes.size(); i += 2) {
        u16 word = bytes[i] << 8 | bytes[i + 1];
        out(", \e[92m{}\e[0m", word);
    }
}

#define INSTRUCTION_FMT(tag) \
    "\e[33m{}\e[36m[\e[95m" tag "\e[36m]\e[0m"

struct InstructionPrinter : InstructionHandler {
    void before_instruction(Context context) override
    {
        switch (context.opcode) {
        case Opcode::EIF:
        case Opcode::ELSE:
        case Opcode::ENDF:
            m_indent_level--;
        default:
            break;
        }
        auto digits = int(AK::log10(float(context.stream.length()))) + 1;
        out("\e[90m{:0{}}:\e[0m{:{}}", context.stream.current_position() - 1, digits, ""sv, m_indent_level * 2);
    }

    void after_instruction(Context context) override
    {
        switch (context.opcode) {
        case Opcode::IF:
        case Opcode::FDEF:
        case Opcode::ELSE:
            m_indent_level++;
        default:
            break;
        }
    }

    void print_instruction(Context context)
    {
        outln(INSTRUCTION_FMT(""), opcode_name(context.opcode));
    }

    void print_instruction(Context context, bool a)
    {
        outln(INSTRUCTION_FMT("{:01b}"), opcode_name(context.opcode), a);
    }

    void print_instruction(Context context, bool a, bool b)
    {
        outln(INSTRUCTION_FMT("{:02b}"), opcode_name(context.opcode), (a << 1) | b);
    }

    void print_instruction(Context context, bool a, bool b, bool c, u8 de)
    {
        outln(INSTRUCTION_FMT("{:05b}"), opcode_name(context.opcode), (a << 4) | (b << 3) | (c << 2) | de);
    }

    void npush_bytes(Context context, ReadonlyBytes values) override
    {
        out(INSTRUCTION_FMT("") " \e[92m{}\e[0m", opcode_name(context.opcode), values.size());
        print_bytes(values);
        outln();
    }

    void npush_words(Context context, ReadonlyBytes values) override
    {
        out(INSTRUCTION_FMT("") " \e[92m{}\e[0m", opcode_name(context.opcode), values.size() / 2);
        print_words(values);
        outln();
    }

    void push_bytes(Context context, ReadonlyBytes values) override
    {
        out(INSTRUCTION_FMT("{:03b}"), opcode_name(context.opcode), values.size() - 1);
        print_bytes(values);
        outln();
    }

    void push_words(Context context, ReadonlyBytes values) override
    {
        out(INSTRUCTION_FMT("{:03b}"), opcode_name(context.opcode), values.size() / 2 - 1);
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

private:
    u32 m_indent_level { 1 };
};

static void print_disassembly(StringView name_format, Optional<ReadonlyBytes> program, u32 code_point = 0)
{
    if (!program.has_value()) {
        out(name_format, code_point);
        outln(": not found");
        return;
    }
    out(name_format, code_point);
    outln(":    ({} bytes)\n", program->size());
    InstructionPrinter printer {};
    InstructionStream stream { printer, *program };
    while (!stream.at_end())
        stream.process_next_instruction();
}

ErrorOr<int> serenity_main(Main::Arguments arguments)
{
    Core::ArgsParser args_parser;

    StringView font_path;
    bool dump_font_program = false;
    bool dump_prep_program = false;
    StringView text;
    args_parser.add_positional_argument(font_path, "Path to font", "FILE");
    args_parser.add_option(dump_font_program, "Disassemble font program (fpgm table)", "disasm-fpgm", 'f');
    args_parser.add_option(dump_prep_program, "Disassemble CVT program (prep table)", "disasm-prep", 'p');
    args_parser.add_option(text, "Disassemble glyph programs", "disasm-glyphs", 'g', "text");
    args_parser.parse(arguments);

    auto font = TRY(OpenType::Font::try_load_from_file(font_path));

    if (dump_font_program)
        print_disassembly("Font program"sv, font->font_program());
    if (dump_prep_program) {
        if (dump_font_program)
            outln();
        print_disassembly("CVT program"sv, font->control_value_program());
    }
    if (!text.is_empty()) {
        Utf8View utf8_view { text };
        bool first = !(dump_font_program || dump_prep_program);
        for (u32 code_point : utf8_view) {
            if (!first)
                outln();
            auto glyph_id = font->glyph_id_for_code_point(code_point);
            print_disassembly("Glyph program codepoint {}"sv, font->glyph_program(glyph_id), code_point);
            first = false;
        }
    }

    return 0;
}
