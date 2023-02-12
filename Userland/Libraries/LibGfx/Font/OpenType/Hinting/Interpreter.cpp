/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Font/OpenType/Hinting/Interpreter.h>

namespace OpenType::Hinting {

ErrorOr<Interpreter> Interpreter::create(size_t max_stack_depth, size_t max_function_defs)
{
    auto stack = TRY(FixedArray<u32>::create(max_stack_depth));
    auto functions = TRY(FixedArray<ReadonlyBytes>::create(max_function_defs));
    HintingData hinting_data {
        .curves = {},
        .zone1 = {},
        .stack = move(stack),
        .functions = move(functions),
        .graphics_state = {}
    };
    return Interpreter(move(hinting_data));
}

uint32_t Interpreter::Stack::pop()
{
    VERIFY(m_top > 0);
    return m_stack[--m_top];
}

void Interpreter::Stack::push(u32 value)
{
    m_stack[m_top++] = value;
}

void Interpreter::Stack::push_byte(u8 value)
{
    // Bytes are expanded to 32 bit quantities by padding the upper bits with zeroes.
    push(value);
}

void Interpreter::Stack::push_word(i16 value)
{
    // Words are sign extended to 32 bits.
    i32 extended_value = value;
    push((u32)extended_value);
}

void Interpreter::handle_NPUSHB(Context context)
{
    handle_PUSHB(context);
}

void Interpreter::handle_NPUSHW(Context context)
{
    handle_NPUSHW(context);
}

void Interpreter::handle_PUSHB(Context context)
{
    auto bytes = context.instruction().values();
    for (u8 byte : bytes)
        m_hinting_data.stack.push_byte(byte);
}

void Interpreter::handle_PUSHW(Context context)
{
    auto span = context.instruction().values();
    for (size_t i = 0; i < span.size(); i += 2)
        m_hinting_data.stack.push_word(span[i] << 8 | span[i + 1]);
}

void Interpreter::handle_FDEF(Context context)
{
    auto& stream = context.stream();
    // First instruction in function:
    auto function_id = m_hinting_data.stack.pop();
    auto fdef_start = stream.current_position();
    stream.jump_to_next(Opcode::ENDF);
    // ENDF marker
    auto fdef_end = stream.current_position();
    auto size = fdef_end - fdef_start;
    auto instructions = stream.take_span(fdef_start, fdef_end);
    m_hinting_data.functions[function_id] = instructions;
}

}
