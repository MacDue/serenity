/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Font/OpenType/Hinting/Interpreter.h>

#define TTF_HINT_DEBUG 1

namespace OpenType::Hinting {

ErrorOr<NonnullRefPtr<Interpreter>> Interpreter::create(Font& font)
{
    auto stack = TRY(FixedArray<u32>::create(font.max_hinting_stack_depth()));
    auto functions = TRY(FixedArray<ReadonlyBytes>::create(font.max_hinting_function_definitions()));
    HintingData hinting_data {
        .curves = {},
        .zone1 = {},
        .stack = move(stack),
        .functions = move(functions),
        .graphics_state = {}
    };
    return TRY(adopt_nonnull_ref_or_enomem(new (nothrow) Interpreter(font, move(hinting_data))));
}

ErrorOr<NonnullOwnPtr<Interpreter::FontInstanceData>> Interpreter::create_font_instance_data(u32 ppem)
{
    auto storage_area = TRY(FixedArray<u32>::create(m_font->max_hinting_storage()));
    // TODO: Find CVT size
    auto cvt = TRY(FixedArray<F26Dot6>::create(0));
    return try_make<FontInstanceData>(ppem, move(storage_area), move(cvt), Zone {});
}

void Interpreter::default_handler(Context context)
{
    dbgln("Unimplemented opcode: {}: {}", context.stream().current_position() - 1, opcode_mnemonic(context.instruction().opcode()));
    TODO();
}

void Interpreter::execute_program(InstructionStream instructions, ProgramContext context)
{
    m_context = context;
    m_hinting_data.stack.clear();
    while (!instructions.at_end())
        instructions.process_next_instruction(*this);
}

uint32_t Interpreter::Stack::pop()
{
    VERIFY(m_top > 0);
    auto value = m_stack[--m_top];
    dbgln_if(TTF_HINT_DEBUG, "Stack: pop {}", value);
    return value;
}

void Interpreter::Stack::push(u32 value)
{
    dbgln_if(TTF_HINT_DEBUG, "Stack: push {}", value);
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

static void log_opcode(InstructionStream::Context context)
{
    dbgln_if(TTF_HINT_DEBUG, "Executing: {}: {}", context.stream().current_position() - 1, opcode_mnemonic(context.instruction().opcode()));
}

void Interpreter::handle_NPUSHB(Context context)
{
    handle_PUSHB(context);
}

void Interpreter::handle_NPUSHW(Context context)
{
    handle_PUSHW(context);
}

void Interpreter::handle_PUSHB(Context context)
{
    log_opcode(context);
    auto bytes = context.instruction().values();
    for (u8 byte : bytes)
        m_hinting_data.stack.push_byte(byte);
}

void Interpreter::handle_PUSHW(Context context)
{
    log_opcode(context);
    auto span = context.instruction().values();
    for (size_t i = 0; i < span.size(); i += 2)
        m_hinting_data.stack.push_word(span[i] << 8 | span[i + 1]);
}

void Interpreter::handle_FDEF(Context context)
{
    log_opcode(context);
    auto& stream = context.stream();
    // First instruction in function:
    auto function_id = m_hinting_data.stack.pop();
    auto fdef_start = stream.current_position();
    stream.jump_passed_next(Opcode::ENDF);
    // ENDF marker
    auto fdef_end = stream.current_position();
    auto instructions = stream.take_span(fdef_start, fdef_end - 1);
    m_hinting_data.functions[function_id] = instructions;
    dbgln_if(TTF_HINT_DEBUG, "Added function definition: {} ({} bytes)", function_id, instructions.size());
}

void Interpreter::handle_CALL(Context context)
{
    log_opcode(context);
    auto function_id = m_hinting_data.stack.pop();
    dbgln_if(TTF_HINT_DEBUG, "Calling function: {}", function_id);
    auto function_bytes = m_hinting_data.functions[function_id];
    if (function_bytes.is_empty()) {
        dbgln_if(TTF_HINT_DEBUG, "Function not found: {}", function_id);
        return;
    }
    InstructionStream function { m_hinting_data.functions[function_id] };
    while (!function.at_end())
        function.process_next_instruction(*this);
    dbgln_if(TTF_HINT_DEBUG, "Call done: {}", function_id);
}

void Interpreter::handle_SVTCA(Context context)
{
    log_opcode(context);
    // TODO: Move point stuff?
    if (context.instruction().a()) {
        m_hinting_data.graphics_state.freedom_vector.set_x(1);
        m_hinting_data.graphics_state.freedom_vector.set_y(0);
    } else {
        m_hinting_data.graphics_state.freedom_vector.set_x(0);
        m_hinting_data.graphics_state.freedom_vector.set_y(1);
    }

    m_hinting_data.graphics_state.projection_vector = m_hinting_data.graphics_state.freedom_vector;
    m_hinting_data.graphics_state.dual_projection_vectors = m_hinting_data.graphics_state.freedom_vector;
    m_hinting_data.graphics_state.projection_dot_free = 1;
}

void Interpreter::handle_MPPEM(Context context)
{
    log_opcode(context);
    m_hinting_data.stack.push(m_context.instance->ppem);
}

void Interpreter::handle_EQ(Context context)
{
    log_opcode(context);
    auto e2 = m_hinting_data.stack.pop();
    auto e1 = m_hinting_data.stack.pop();
    m_hinting_data.stack.push(e1 == e2);
}

void Interpreter::handle_WS(Context context)
{
    log_opcode(context);
    auto value = m_hinting_data.stack.pop();
    auto location = m_hinting_data.stack.pop();
    m_context.instance->storage_area[location] = value;
}

void Interpreter::handle_GETINFO(Context context)
{
    log_opcode(context);
    // Pretend to be FreeType:
    u32 result = 0;
    auto selector = m_hinting_data.stack.pop();

    // Version
    if (selector & 0x00000001)
        result = 40; // v40

    // Glyph rotation
    if (selector & 0x00000002)
        // <Always false>

        // Glyph stretched
        if (selector & 0x00000004)
            // <Always false>

            // ClearType enabled (always true in FreeType)
            if (selector & 0x00000040)
                result |= (1 << 13);

    m_hinting_data.stack.push(result);
}

void Interpreter::handle_LTEQ(Context context)
{
    log_opcode(context);
    auto e2 = m_hinting_data.stack.pop();
    auto e1 = m_hinting_data.stack.pop();
    m_hinting_data.stack.push(e1 <= e2);
}

void Interpreter::handle_GTEQ(Context context)
{
    log_opcode(context);
    auto e2 = m_hinting_data.stack.pop();
    auto e1 = m_hinting_data.stack.pop();
    m_hinting_data.stack.push(e1 >= e2);
}

void Interpreter::handle_AND(Context context)
{
    log_opcode(context);
    auto e2 = m_hinting_data.stack.pop();
    auto e1 = m_hinting_data.stack.pop();
    m_hinting_data.stack.push(e1 && e2);
}

void Interpreter::handle_IF(Context context)
{
    log_opcode(context);
    auto& stream = context.stream();
    auto jump_passed_next_else_or_eif = [&] {
        unsigned if_nesting = 0;
        Opcode end_opcode {};
        stream.jump_passed([&](Opcode opcode) {
            end_opcode = opcode;
            if (opcode == Opcode::IF)
                ++if_nesting;
            else if (if_nesting == 0)
                return opcode == Opcode::ELSE || opcode == Opcode::EIF;
            else if (opcode == Opcode::EIF)
                --if_nesting;
            return false;
        });
        return end_opcode;
    };
    auto e = m_hinting_data.stack.pop();
    if (!e) {
        // Test failed jump to after the first ELSE[] or EIF[].
        if (jump_passed_next_else_or_eif() == Opcode::EIF)
            return;
    }
    // Run then/else body:
    while (true) {
        auto opcode = stream.peek();
        if (opcode == Opcode::ELSE) {
            stream.skip_instruction();
            jump_passed_next_else_or_eif();
            return;
        }
        if (opcode == Opcode::EIF) {
            stream.skip_instruction();
            return;
        }
        stream.process_next_instruction(*this);
    }
}

void Interpreter::handle_RS(Context context)
{
    log_opcode(context);
    auto location = m_hinting_data.stack.pop();
    m_hinting_data.stack.push(m_context.instance->storage_area[location]);
}

void Interpreter::handle_ADD(Context context)
{
    log_opcode(context);
    auto n1 = F26Dot6::create_raw(m_hinting_data.stack.pop());
    auto n2 = F26Dot6::create_raw(m_hinting_data.stack.pop());
    m_hinting_data.stack.push((n2 + n1).raw());
}

void Interpreter::handle_MUL(Context context)
{
    log_opcode(context);
    auto n1 = F26Dot6::create_raw(m_hinting_data.stack.pop());
    auto n2 = F26Dot6::create_raw(m_hinting_data.stack.pop());
    m_hinting_data.stack.push((n2 * n1).raw());
}

void Interpreter::handle_GT(Context context)
{
    log_opcode(context);
    auto e2 = m_hinting_data.stack.pop();
    auto e1 = m_hinting_data.stack.pop();
    m_hinting_data.stack.push(e1 > e2);
}

void Interpreter::handle_LT(Context context)
{
    log_opcode(context);
    auto e2 = m_hinting_data.stack.pop();
    auto e1 = m_hinting_data.stack.pop();
    m_hinting_data.stack.push(e1 < e2);
}

void Interpreter::handle_OR(Context context)
{
    log_opcode(context);
    auto e2 = m_hinting_data.stack.pop();
    auto e1 = m_hinting_data.stack.pop();
    m_hinting_data.stack.push(e1 || e2);
}

void Interpreter::handle_NEQ(Context context)
{
    log_opcode(context);
    auto e2 = m_hinting_data.stack.pop();
    auto e1 = m_hinting_data.stack.pop();
    m_hinting_data.stack.push(e1 != e2);
}

void Interpreter::handle_SCANCTRL(Context context)
{
    log_opcode(context);
    auto flags = m_hinting_data.stack.pop();
    u8 threshold = flags & 0xff;

    if (threshold == 0xff) {
        m_hinting_data.graphics_state.scan_control = true;
    } else if (threshold == 0x00) {
        m_hinting_data.graphics_state.scan_control = false;
    } else {
        // Bit 8: Set dropout_control to TRUE if other conditions do not block and ppem is less than or equal to the threshold value.
        if ((flags & (1 << 8)) && m_context.instance->ppem <= threshold) {
            m_hinting_data.graphics_state.scan_control = true;
        }
        // Bit 11: Set dropout_control to FALSE unless ppem is less than or equal to the threshold value.
        if (flags & (1 << 11) && m_context.instance->ppem > threshold) {
            m_hinting_data.graphics_state.scan_control = false;
        }
        // TODO: other bits
    }
}

void Interpreter::handle_SCANTYPE(Context context)
{
    log_opcode(context);
    m_hinting_data.graphics_state.scan_type = m_hinting_data.stack.pop();
}

void Interpreter::handle_SCVTCI(Context context)
{
    log_opcode(context);
    m_hinting_data.graphics_state.control_value_cut_in = m_hinting_data.stack.pop();
}

void Interpreter::delta_c()
{
    // This is just a stub that pops the stack values.
    auto n = m_hinting_data.stack.pop();
    while (n > 0) {
        [[maybe_unused]] auto cvt_index = m_hinting_data.stack.pop();
        [[maybe_unused]] auto arg = m_hinting_data.stack.pop();
        // TODO: Apply shift to CVT
        --n;
    }
}

void Interpreter::handle_DELTAC1(Context context)
{
    log_opcode(context);
    dbgln("TODO: DELTAC1");
    delta_c();
}

void Interpreter::handle_DELTAC2(Context context)
{
    log_opcode(context);
    dbgln("TODO: DELTAC2");
    delta_c();
}

void Interpreter::handle_DELTAC3(Context context)
{
    log_opcode(context);
    dbgln("TODO: DELTAC3");
    delta_c();
}

void Interpreter::handle_POP(Context context)
{
    log_opcode(context);
    (void)m_hinting_data.stack.pop();
}

void Interpreter::handle_RCVT(Context context)
{
    log_opcode(context);
    dbgln("TODO: RCVT");
    [[maybe_unused]] auto cvt_index = m_hinting_data.stack.pop();
    m_hinting_data.stack.push(0);
}

void Interpreter::handle_RTG(Context context)
{
    log_opcode(context);
    m_hinting_data.graphics_state.round_state = 1;
}

void Interpreter::handle_ROUND(Context context)
{
    log_opcode(context);
    dbgln("TODO: ROUND");
    auto n1 = m_hinting_data.stack.pop();
    auto n2 = /*some_how_round:*/ n1;
    m_hinting_data.stack.push(n2);
}

void Interpreter::handle_SWAP(Context context)
{
    log_opcode(context);
    auto e2 = m_hinting_data.stack.pop();
    auto e1 = m_hinting_data.stack.pop();
    m_hinting_data.stack.push(e2);
    m_hinting_data.stack.push(e1);
}

void Interpreter::handle_DUP(Context context)
{
    log_opcode(context);
    auto e = m_hinting_data.stack.pop();
    m_hinting_data.stack.push(e);
    m_hinting_data.stack.push(e);
}

void Interpreter::handle_ABS(Context context)
{
    log_opcode(context);
    auto n = F26Dot6::create_raw(m_hinting_data.stack.pop());
    if (n < F26Dot6(0))
        n = F26Dot6(0) - n;
    m_hinting_data.stack.push(n.raw());
}

void Interpreter::handle_RUTG(Context context)
{
    log_opcode(context);
    m_hinting_data.graphics_state.round_state = 4;
}

void Interpreter::handle_ROLL(Context context)
{
    log_opcode(context);
    auto a = m_hinting_data.stack.pop();
    auto b = m_hinting_data.stack.pop();
    auto c = m_hinting_data.stack.pop();
    m_hinting_data.stack.push(b);
    m_hinting_data.stack.push(a);
    m_hinting_data.stack.push(c);
}

void Interpreter::handle_WCVTP(Context context)
{
    log_opcode(context);
    dbgln("TODO: WCVTP");
    [[maybe_unused]] auto value = F26Dot6::create_raw(m_hinting_data.stack.pop());
    [[maybe_unused]] auto location = m_hinting_data.stack.pop();
}

void Interpreter::handle_SDB(Context context)
{
    log_opcode(context);
    m_hinting_data.graphics_state.delta_base = m_hinting_data.stack.pop();
}

}
