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
#define BASE_INSTRUCTION_FMT(tag_fmt) "\e[33m{}\e[36m[\e[95m" tag_fmt "\e[36m]\e[0m"
#define WITH_TAG_INSTRUCTION_FMT BASE_INSTRUCTION_FMT("{:0{}b}")
#define INSTRUCTION_FMT BASE_INSTRUCTION_FMT("")

struct InstructionPrinter : InstructionHandler {
    void before_operation(InstructionStream& stream, Opcode opcode) override
    {
        if (opcode == Opcode::FDEF && stream.current_position() > 1)
            outln();
        switch (opcode) {
        case Opcode::EIF:
        case Opcode::ELSE:
        case Opcode::ENDF:
            m_indent_level--;
        default:
            break;
        }
        auto digits = int(AK::log10(float(stream.length()))) + 1;
        out("\e[90m{:0{}}:\e[0m{:{}}", stream.current_position() - 1, digits, ""sv, m_indent_level * 2);
    }

    void after_operation(InstructionStream&, Opcode opcode) override
    {
        switch (opcode) {
        case Opcode::IF:
        case Opcode::ELSE:
        case Opcode::FDEF:
            m_indent_level++;
        default:
            break;
        }
    }

    void default_handler(Context context) override
    {
        auto instruction = context.instruction();
        auto name = opcode_mnemonic(instruction.opcode());
        if (instruction.flag_bits() > 0)
            out(WITH_TAG_INSTRUCTION_FMT, name, to_underlying(instruction.opcode()) & ((1 << instruction.flag_bits()) - 1), instruction.flag_bits());
        else
            out(INSTRUCTION_FMT, name);
        switch (instruction.opcode()) {
        case Opcode::PUSHB... Opcode::PUSHB_MAX:
        case Opcode::NPUSHB... Opcode::NPUSHB_MAX:
            print_bytes(instruction.values());
            break;
        case Opcode::PUSHW... Opcode::PUSHW_MAX:
        case Opcode::NPUSHW... Opcode::NPUSHW_MAX:
            print_words(instruction.values());
        default:
            break;
        }
        outln();
    }

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
            print_disassembly("Glyph program for codepoint {}"sv, font->glyph_program(glyph_id), code_point);
            first = false;
        }
    }

    return 0;
}
