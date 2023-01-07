/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Format.h>
#include <LibGfx/Font/OpenType/Hinting/InstructionPrinter.h>

namespace OpenType::Hinting {

void InstructionPrinter::print_instruction(Context context)
{
    outln("{}[]", opcode_name(context.opcode));
}

void InstructionPrinter::print_instruction(Context context, bool a)
{
    outln("{}[{01b}]", opcode_name(context.opcode), a);
}

void InstructionPrinter::print_instruction(Context context, bool a, bool b)
{
    outln("{}[{02b}]", opcode_name(context.opcode), (a << 1) | b);
}

void InstructionPrinter::print_instruction(Context context, bool a, bool b, bool c, u8 de)
{
    outln("{}[{05b}]", opcode_name(context.opcode), (a << 4) | (b << 3) | (c << 2) | de);
}

static void print_bytes(ReadonlyBytes bytes)
{
    for (auto value : bytes)
        out(", {}", value);
}

static void print_words(ReadonlyBytes bytes)
{
    for (size_t i = 0; i < bytes.size(); i++) {
        u16 word = bytes[i] << 8 | bytes[i + 1];
        out(", {}", word);
    }
}

void InstructionPrinter::npush_bytes(Context context, ReadonlyBytes values)
{
    out("{}[] {}", opcode_name(context.opcode), values.size());
    print_bytes(values);
    outln();
}

void InstructionPrinter::npush_words(Context context, ReadonlyBytes values)
{
    out("{}[] {}", opcode_name(context.opcode), values.size());
    print_words(values);
    outln();
}

void InstructionPrinter::push_bytes(Context context, ReadonlyBytes values)
{
    out("{}[{03b}]", opcode_name(context.opcode), values.size());
    print_bytes(values);
    outln();
}

void InstructionPrinter::push_words(Context context, ReadonlyBytes values)
{
    out("{}[{03b}]", opcode_name(context.opcode), values.size());
    print_words(values);
    outln();
}

}
