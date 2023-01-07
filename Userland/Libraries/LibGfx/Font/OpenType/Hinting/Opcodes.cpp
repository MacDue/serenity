/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Font/OpenType/Hinting/Opcodes.h>

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
    case Opcode::WCVTP:
        return m_handler.write_control_value_table_px(*this);
    case Opcode::WCVTF:
        return m_handler.write_control_value_table_fuints(*this);
    case Opcode::RCVT:
        return m_handler.read_control_value_table(*this);
    case Opcode::SVTCA... Opcode::SVTCA_MAX:
        return m_handler.set_freedom_and_projection_vectors_to_coordinate_axis(*this, to_underlying(opcode) & 1);
    case Opcode::SPVTCA... Opcode::SPVTCA_MAX:
        return m_handler.set_projection_vector_to_coordinate_axis(*this, to_underlying(opcode) & 1);
    case Opcode::SFVTCA... Opcode::SFVTCA_MAX:
        return m_handler.set_freedom_vector_to_coordinate_axis(*this, to_underlying(opcode) & 1);
    case Opcode::SPVTL... Opcode::SPVTL_MAX:
        return m_handler.set_projection_vector_to_line(*this, to_underlying(opcode) & 1);
    case Opcode::SFVTL... Opcode::SFVTL_MAX:
        return m_handler.set_freedom_vector_vector_to_line(*this, to_underlying(opcode) & 1);
    case Opcode::SFVTPV:
        return m_handler.set_freedom_vector_to_projection_vector(*this);
    case Opcode::SDPVTL... Opcode::SDPVTL_MAX:
        return m_handler.set_dual_projection_vector_to_line(*this, to_underlying(opcode) & 1);
    case Opcode::SPVFS:
        return m_handler.set_projection_vector_from_stack(*this);
    case Opcode::SFVFS:
        return m_handler.set_freedom_vector_from_stack(*this);
    case Opcode::GPV:
        return m_handler.get_projection_vector(*this);
    case Opcode::GFV:
        return m_handler.get_freedom_vector(*this);
    case Opcode::SRP0... Opcode::SRP2:
        return m_handler.set_refrence_point(*this, to_underlying(opcode) & 0xf);
    case Opcode::SZP0... Opcode::SZP2:
        return m_handler.set_zone_pointer(*this, to_underlying(opcode) - to_underlying(Opcode::SZP0));
    case Opcode::SZPS:
        return m_handler.set_zone_pointers(*this);
    case Opcode::RTHG:
        return m_handler.round_to_half_grid(*this);
    case Opcode::RTG:
        return m_handler.round_to_grid(*this);
    case Opcode::RTDG:
        return m_handler.round_to_double_grid(*this);
    case Opcode::RDTG:
        return m_handler.round_down_to_grid(*this);
    case Opcode::RUTG:
        return m_handler.round_up_to_grid(*this);
    case Opcode::ROFF:
        return m_handler.round_off(*this);
    case Opcode::SROUND:
        return m_handler.super_round(*this);
    case Opcode::S45ROUND:
        return m_handler.super_round_45_degrees(*this);
    case Opcode::SLOOP:
        return m_handler.set_loop_variable(*this);
    case Opcode::SMD:
        return m_handler.set_minimum_distance(*this);
    case Opcode::INSTCTRL:
        return m_handler.instruction_execution_control(*this);
    case Opcode::SCANCTRL:
        return m_handler.scan_conversion_control(*this);
    case Opcode::SCANTYPE:
        return m_handler.scantype(*this);
    case Opcode::SCVTCI:
        return m_handler.set_control_value_table_in(*this);
    case Opcode::SSWCI:
        return m_handler.set_single_width_cut_in(*this);
    case Opcode::SSW:
        return m_handler.set_single_width(*this);
    case Opcode::FLIPON:
        return m_handler.set_auto_flip_boolean(*this, true);
    case Opcode::FLIPOFF:
        return m_handler.set_auto_flip_boolean(*this, false);
    case Opcode::SANGW:
        return m_handler.set_angle_weight(*this);
    case Opcode::SDB:
        return m_handler.set_delta_base(*this);
    case Opcode::SDS:
        return m_handler.set_delta_shift(*this);
    case Opcode::GC... Opcode::GC_MAX:
        return m_handler.get_coordinate_projected_onto_projection_vector(*this, to_underlying(opcode) & 1);
    case Opcode::SCFS:
        return m_handler.set_coordinate_from_stack_using_projection_and_freedom_vectors(*this);
    case Opcode::MD... Opcode::MD_MAX:
        return m_handler.measure_distance(*this, to_underlying(opcode) & 1);
    case Opcode::MPPEM:
        return m_handler.measure_pixels_per_em(*this);
    case Opcode::MPS:
        return m_handler.measure_point_size(*this);
    case Opcode::FLIPPT:
        return m_handler.flip_point(*this);
    case Opcode::FLIPRGON:
        return m_handler.flip_range_on(*this);
    case Opcode::FLIPRGOFF:
        return m_handler.flip_range_off(*this);
    case Opcode::SHP... Opcode::SHP_MAX:
        return m_handler.shift_point_by_last_point(*this, to_underlying(opcode) & 1);
    case Opcode::SHC... Opcode::SHC_MAX:
        return m_handler.shift_contour_by_last_point(*this, to_underlying(opcode) & 1);
    case Opcode::SHZ... Opcode::SHZ_MAX:
        return m_handler.shift_zone_by_last_point(*this, to_underlying(opcode) & 1);
    case Opcode::SHPIX:
        return m_handler.shift_point_by_pixel_amount(*this);
    case Opcode::MSIRP... Opcode::MSIRP_MAX:
        return m_handler.move_stack_indirect_relative_point(*this, to_underlying(opcode) & 1);
    case Opcode::MDAP... Opcode::MDAP_MAX:
        return m_handler.move_direct_absolute_point(*this, to_underlying(opcode) & 1);
    case Opcode::MIAP... Opcode::MIAP_MAX:
        return m_handler.move_indirect_absolute_point(*this, to_underlying(opcode) & 1);
    case Opcode::MDRP... Opcode::MDRP_MAX: {
        auto opcode_byte = to_underlying(opcode);
        return m_handler.move_direct_relative_point(*this,
            opcode_byte & 0b10000, opcode_byte & 0b1000, opcode_byte & 0b100, opcode_byte & 0b11);
    }
    case Opcode::MIRP... Opcode::MIRP_MAX: {
        auto opcode_byte = to_underlying(opcode);
        return m_handler.move_indirect_relative_point(*this,
            opcode_byte & 0b10000, opcode_byte & 0b1000, opcode_byte & 0b100, opcode_byte & 0b11);
    }
    case Opcode::ALIGNRP:
        return m_handler.align_relative_point(*this);
    case Opcode::ISECT:
        return m_handler.intersect_lines(*this);
    case Opcode::ALIGNPTS:
        return m_handler.align_points(*this);
    case Opcode::IP:
        return m_handler.interpolate_point_by_last_relative_stretch(*this);
    case Opcode::UTP:
        return m_handler.untouch_point(*this);
    case Opcode::IUP... Opcode::IUP_MAX:
        return m_handler.interpolate_untouched_points_through_outline(*this, to_underlying(opcode) & 1);
    case Opcode::DELTAP1:
        return m_handler.delta_exception_p(*this, 1);
    case Opcode::DELTAP2:
        return m_handler.delta_exception_p(*this, 2);
    case Opcode::DELTAP3:
        return m_handler.delta_exception_p(*this, 3);
    case Opcode::DELTAC1... Opcode::DELTAC3:
        return m_handler.delta_exception_c(*this, to_underlying(opcode) - to_underlying(Opcode::DELTAC1));
    case Opcode::DUP:
        return m_handler.stack_dup(*this);
    case Opcode::POP:
        return m_handler.stack_pop(*this);
    case Opcode::CLEAR:
        return m_handler.stack_clear(*this);
    case Opcode::SWAP:
        return m_handler.stack_swap(*this);
    case Opcode::DEPTH:
        return m_handler.stack_depth(*this);
    case Opcode::CINDEX:
        return m_handler.stack_copy_indexed_element(*this);
    case Opcode::MINDEX:
        return m_handler.stack_move_indexed_element(*this);
    case Opcode::ROLL:
        return m_handler.stack_roll_three_elements(*this);
    case Opcode::IF:
        return m_handler.if_test(*this);
    case Opcode::ELSE:
        return m_handler.else_case(*this);
    case Opcode::EIF:
        return m_handler.end_if(*this);
    case Opcode::JROT:
        return m_handler.jump_relative_on_true(*this);
    case Opcode::JMPR:
        return m_handler.jump(*this);
    case Opcode::JROF:
        return m_handler.jump_relative_on_false(*this);
    case Opcode::LT:
        return m_handler.less_than(*this);
    case Opcode::LTEQ:
        return m_handler.less_than_or_equal(*this);
    case Opcode::GT:
        return m_handler.greater_than(*this);
    case Opcode::GTEQ:
        return m_handler.greater_than_or_equal(*this);
    case Opcode::EQ:
        return m_handler.equal(*this);
    case Opcode::NEQ:
        return m_handler.not_equal(*this);
    case Opcode::ODD:
        return m_handler.odd(*this);
    case Opcode::EVEN:
        return m_handler.even(*this);
    case Opcode::AND:
        return m_handler.logical_and(*this);
    case Opcode::OR:
        return m_handler.logical_or(*this);
    case Opcode::NOT:
        return m_handler.logical_not(*this);
    case Opcode::ADD:
        return m_handler.add(*this);
    case Opcode::SUB:
        return m_handler.subtract(*this);
    case Opcode::DIV:
        return m_handler.divide(*this);
    case Opcode::MUL:
        return m_handler.multiply(*this);
    case Opcode::ABS:
        return m_handler.absolute_value(*this);
    case Opcode::NEG:
        return m_handler.negate(*this);
    case Opcode::FLOOR:
        return m_handler.floor(*this);
    case Opcode::CEILING:
        return m_handler.ceiling(*this);
    case Opcode::MAX:
        return m_handler.max(*this);
    case Opcode::MIN:
        return m_handler.min(*this);
    case Opcode::ROUND... Opcode::ROUND_MAX: {
        auto opcode_byte = to_underlying(opcode);
        return m_handler.round(*this, opcode_byte & 0b10, opcode_byte & 0b01);
    }
    case Opcode::NROUND... Opcode::NROUND_MAX: {
        auto opcode_byte = to_underlying(opcode);
        return m_handler.no_round(*this, opcode_byte & 0b10, opcode_byte & 0b01);
    }
    case Opcode::FDEF:
        return m_handler.function_definition(*this);
    case Opcode::ENDF:
        return m_handler.end_function_definition(*this);
    case Opcode::CALL:
        return m_handler.call_function(*this);
    case Opcode::LOOPCALL:
        return m_handler.loop_and_call_function(*this);
    case Opcode::IDEF:
        return m_handler.instruction_definition(*this);
    case Opcode::DEBUG:
        return m_handler.debug_call(*this);
    case Opcode::GETINFO:
        return m_handler.get_information(*this);
    case Opcode::GETVARIATION:
        return m_handler.get_variation(*this);
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
