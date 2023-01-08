/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/IntegralMath.h>
#include <AK/ScopeGuard.h>
#include <LibGfx/Font/OpenType/Hinting/Opcodes.h>

namespace OpenType::Hinting {

StringView opcode_name(Opcode opcode)
{
    switch (to_underlying(opcode)) {
#define __ENUMERATE_OPENTYPE_OPCODES(mnemonic, range_start, range_end) \
    case range_start ... range_end:                                    \
        return #mnemonic##sv;
        ENUMERATE_OPENTYPE_OPCODES
#undef __ENUMERATE_OPENTYPE_OPCODES
    }
    VERIFY_NOT_REACHED();
}

static u8 flag_bits(Opcode opcode)
{
    switch (to_underlying(opcode)) {
#define __ENUMERATE_OPENTYPE_OPCODES(mnemonic, range_start, range_end) \
    case range_start ... range_end:                                    \
        return AK::ceil_log2(range_end - range_start);
        ENUMERATE_OPENTYPE_OPCODES
#undef __ENUMERATE_OPENTYPE_OPCODES
    }
    VERIFY_NOT_REACHED();
}

Instruction::Instruction(Opcode opcode, ReadonlyBytes values)
    : m_opcode(opcode)
    , m_values(m_values)
    , m_flag_bits(flag_bits(opcode))
{
}

bool Instruction::a() const
{
    return to_underlying(m_opcode) >> (m_flag_bits - 1);
}

bool Instruction::b() const
{
    return to_underlying(m_opcode) >> (m_flag_bits - 2);
}

bool Instruction::c() const
{
    return to_underlying(m_opcode) >> (m_flag_bits - 3);
}

bool Instruction::d() const
{
    return to_underlying(m_opcode) >> (m_flag_bits - 4);
}

bool Instruction::e() const
{
    return to_underlying(m_opcode) >> (m_flag_bits - 5);
}

void InstructionStream::process_next_instruction()
{
    auto opcode = static_cast<Opcode>(next_byte());
    auto& stream = *this;
    m_handler.before_operation(stream, opcode);
    ScopeGuard after = [=, this] {
        m_handler.after_operation(stream, opcode);
    };
    switch (opcode) {
    case Opcode::NPUSHB: {
        auto n = next_byte();
        auto values = take_n_bytes(n);
        return m_handler.handle_NPUSHB({ { opcode, values }, stream });
    }
    case Opcode::NPUSHW: {
        auto n = next_byte();
        auto values = take_n_bytes(n * 2);
        return m_handler.handle_NPUSHW({ { opcode, values }, stream });
    }
    case Opcode::PUSHB... Opcode::PUSHB_MAX: {
        auto n = (to_underlying(opcode) & 0b111) + 1;
        auto values = take_n_bytes(n);
        return m_handler.handle_PUSHB({ { opcode, values }, stream });
    }
    case Opcode::PUSHW... Opcode::PUSHW_MAX: {
        auto n = (to_underlying(opcode) & 0b111) + 1;
        auto values = take_n_bytes(n * 2);
        return m_handler.handle_PUSHB({ { opcode, values }, stream });
    }
    }
    switch (to_underlying(opcode)) {
#define __ENUMERATE_OPENTYPE_OPCODES(mnemonic, range_start, range_end) \
    case range_start ... range_end:                                    \
        return m_handler.handle_##mnemonic({ { opcode }, stream });
        ENUMERATE_OPENTYPE_OPCODES
#undef __ENUMERATE_OPENTYPE_OPCODES
    }
    VERIFY_NOT_REACHED();
}

u8 InstructionStream::next_byte()
{
    VERIFY(!at_end());
    return m_bytes[m_byte_index++];
}

ReadonlyBytes InstructionStream::take_n_bytes(size_t n)
{
    VERIFY(m_byte_index + n < m_bytes.size());
    auto bytes = m_bytes.slice(m_byte_index, n);
    m_byte_index += n;
    return bytes;
}

bool InstructionStream::at_end() const
{
    return m_byte_index >= m_bytes.size();
}

void InstructionStream::jump_to_next(Opcode)
{
    TODO();
}

}
