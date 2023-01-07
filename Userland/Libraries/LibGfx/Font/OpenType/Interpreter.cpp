/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Font/OpenType/Interpreter.h>

namespace OpenType::Hinting {

StringView opcode_name(Opcode opcode)
{
    // Awkward cases (multiple opcodes with the same mnemonic):
    switch (opcode) {
    case Opcode::PUSHB... Opcode::PUSHB_MAX:
        return "PUSHB"sv;
    case Opcode::PUSHW... Opcode::PUSHW_MAX:
        return "PUSHW"sv;
    case Opcode::SVTCA... Opcode::SVTCA_MAX:
        return "SVTCA"sv;
    case Opcode::SPVTCA... Opcode::SPVTCA_MAX:
        return "SPVTCA"sv;
    case Opcode::SFVTCA... Opcode::SFVTCA_MAX:
        return "SFVTCA"sv;
    case Opcode::SPVTL... Opcode::SPVTL_MAX:
        return "SPVTL"sv;
    case Opcode::SFVTL... Opcode::SFVTL_MAX:
        return "SFVTL"sv;
    case Opcode::SDPVTL... Opcode::SDPVTL_MAX:
        return "SDPVTL"sv;
    case Opcode::GC... Opcode::GC_MAX:
        return "GC"sv;
    case Opcode::MD... Opcode::MD_MAX:
        return "MD"sv;
    case Opcode::SHP... Opcode::SHP_MAX:
        return "SHP"sv;
    case Opcode::SHC... Opcode::SHC_MAX:
        return "SHC"sv;
    case Opcode::SHZ... Opcode::SHZ_MAX:
        return "SHZ"sv;
    case Opcode::MSIRP... Opcode::MSIRP_MAX:
        return "MSIRP"sv;
    case Opcode::MDAP... Opcode::MDAP_MAX:
        return "MDAP"sv;
    case Opcode::MIAP... Opcode::MIAP_MAX:
        return "MIAP"sv;
    case Opcode::MDRP... Opcode::MDRP_MAX:
        return "MDRP"sv;
    case Opcode::MIRP... Opcode::MIRP_MAX:
        return "MIRP"sv;
    case Opcode::IUP... Opcode::IUP_MAX:
        return "IUP"sv;
    case Opcode::ROUND... Opcode::ROUND_MAX:
        return "ROUND"sv;
    case Opcode::NROUND... Opcode::NROUND_MAX:
        return "NROUND"sv;
    default:
        break;
    }
    // Everything else:
    switch (opcode) {
#define __ENUMERATE_OPENTYPE_OPCODES(opcode, _) \
    return #opcode##sv;
        ENUMERATE_OPENTYPE_OPCODES
#undef __ENUMERATE_OPENTYPE_OPCODES
    }
}

void InstructionStream::process_next_instruction()
{
    auto opcode = next_opcode();
    switch (opcode) {
    case Opcode::NPUSHB: {
        auto n = next_byte();
        auto values = take_n_bytes(n);
        return m_handler.npush_bytes(*this, values);
    }
    case Opcode::NPUSHW: {
        auto n = next_byte();
        auto values = take_n_bytes(n * 2);
        return m_handler.npush_words(*this, values);
    }
    case Opcode::PUSHB... Opcode::PUSHB_MAX: {
        auto n = (to_underlying(opcode) & 0b111) + 1;
        auto values = take_n_bytes(n);
        return m_handler.push_bytes(*this, values);
    }
    case Opcode::PUSHW... Opcode::PUSHW_MAX: {
        auto n = (to_underlying(opcode) & 0b111) + 1;
        auto values = take_n_bytes(n * 2);
        return m_handler.push_words(*this, values);
    }
    case Opcode::RS:
        return m_handler.read_store(*this);
    case Opcode::WS:
        return m_handler.write_store(*this);
    }
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
}

}
