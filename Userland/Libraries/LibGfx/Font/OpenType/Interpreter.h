/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/FixedPoint.h>
#include <AK/Span.h>
#include <LibGfx/Point.h>

namespace OpenType::Hinting {

using F2Dot14 = FixedPoint<30, u16>;
using F2Dot30 = FixedPoint<30, u32>;
using F26Dot6 = FixedPoint<6, u32>;

#define ENUMERATE_OPENTYPE_OPCODES                      \
    /* Pushing data onto the interpreter stack: */      \
    __ENUMERATE_OPENTYPE_OPCODES(NPUSHB, 0x40)          \
    __ENUMERATE_OPENTYPE_OPCODES(NPUSHW, 0x41)          \
    __ENUMERATE_OPENTYPE_OPCODES(PUSHB, 0xB0)           \
    __ENUMERATE_OPENTYPE_OPCODES(PUSHB_MAX, 0xB7)       \
    __ENUMERATE_OPENTYPE_OPCODES(PUSHW, 0xB8)           \
    __ENUMERATE_OPENTYPE_OPCODES(PUSHW_MAX, 0xBF)       \
    /* Managing the Storage Area */                     \
    __ENUMERATE_OPENTYPE_OPCODES(RS, 0x43)              \
    __ENUMERATE_OPENTYPE_OPCODES(WS, 0x42)              \
    /* Managing the Control Value Table */              \
    __ENUMERATE_OPENTYPE_OPCODES(WCVTP, 0x44)           \
    __ENUMERATE_OPENTYPE_OPCODES(WCVTF, 0x70)           \
    __ENUMERATE_OPENTYPE_OPCODES(RCVT, 0x45)            \
    /* Managing the Graphics State */                   \
    __ENUMERATE_OPENTYPE_OPCODES(SVTCA, 0x00)           \
    __ENUMERATE_OPENTYPE_OPCODES(SVTCA_MAX, 0x1)        \
    __ENUMERATE_OPENTYPE_OPCODES(SPVTCA, 0x02)          \
    __ENUMERATE_OPENTYPE_OPCODES(SPVTCA_MAX, 0x3)       \
    __ENUMERATE_OPENTYPE_OPCODES(SFVTCA, 0x04)          \
    __ENUMERATE_OPENTYPE_OPCODES(SFVTCA_MAX, 0x05)      \
    __ENUMERATE_OPENTYPE_OPCODES(SPVTL, 0x06)           \
    __ENUMERATE_OPENTYPE_OPCODES(SPVTL_MAX, 0x07)       \
    __ENUMERATE_OPENTYPE_OPCODES(SFVTL, 0x08)           \
    __ENUMERATE_OPENTYPE_OPCODES(SFVTL_MAX, 0x09)       \
    __ENUMERATE_OPENTYPE_OPCODES(SFVTPV, 0x0E)          \
    __ENUMERATE_OPENTYPE_OPCODES(SDPVTL, 0x86)          \
    __ENUMERATE_OPENTYPE_OPCODES(SDPVTL_MAX, 0x87)      \
    __ENUMERATE_OPENTYPE_OPCODES(SPVFS, 0x0A)           \
    __ENUMERATE_OPENTYPE_OPCODES(SFVFS, 0x0B)           \
    __ENUMERATE_OPENTYPE_OPCODES(GPV, 0x0C)             \
    __ENUMERATE_OPENTYPE_OPCODES(GFV, 0x0D)             \
    __ENUMERATE_OPENTYPE_OPCODES(SRP0, 0x10)            \
    __ENUMERATE_OPENTYPE_OPCODES(SRP1, 0x11)            \
    __ENUMERATE_OPENTYPE_OPCODES(SRP2, 0x12)            \
    __ENUMERATE_OPENTYPE_OPCODES(SZP0, 0x13)            \
    __ENUMERATE_OPENTYPE_OPCODES(SZP1, 0x14)            \
    __ENUMERATE_OPENTYPE_OPCODES(SZP2, 0x15)            \
    __ENUMERATE_OPENTYPE_OPCODES(SZPS, 0x16)            \
    __ENUMERATE_OPENTYPE_OPCODES(RTHG, 0x19)            \
    __ENUMERATE_OPENTYPE_OPCODES(RTG, 0x18)             \
    __ENUMERATE_OPENTYPE_OPCODES(RTDG, 0x3D)            \
    __ENUMERATE_OPENTYPE_OPCODES(RDTG, 0x7D)            \
    __ENUMERATE_OPENTYPE_OPCODES(RUTG, 0x7C)            \
    __ENUMERATE_OPENTYPE_OPCODES(ROFF, 0x7A)            \
    __ENUMERATE_OPENTYPE_OPCODES(SROUND, 0x76)          \
    __ENUMERATE_OPENTYPE_OPCODES(S45ROUND, 0x77)        \
    __ENUMERATE_OPENTYPE_OPCODES(SLOOP, 0x17)           \
    __ENUMERATE_OPENTYPE_OPCODES(SMD, 0x1A)             \
    __ENUMERATE_OPENTYPE_OPCODES(INSTCTRL, 0x8E)        \
    __ENUMERATE_OPENTYPE_OPCODES(SCANCTRL, 0x85)        \
    __ENUMERATE_OPENTYPE_OPCODES(SCANTYPE, 0x8D)        \
    __ENUMERATE_OPENTYPE_OPCODES(SCVTCI, 0x1D)          \
    __ENUMERATE_OPENTYPE_OPCODES(SSWCI, 0x1E)           \
    __ENUMERATE_OPENTYPE_OPCODES(SSW, 0x1F)             \
    __ENUMERATE_OPENTYPE_OPCODES(FLIPON, 0x4D)          \
    __ENUMERATE_OPENTYPE_OPCODES(FLIPOFF, 0x4E)         \
    __ENUMERATE_OPENTYPE_OPCODES(SANGW, 0x7E)           \
    __ENUMERATE_OPENTYPE_OPCODES(SDB, 0x5E)             \
    __ENUMERATE_OPENTYPE_OPCODES(SDS, 0x5F)             \
    /* Reading and writing data */                      \
    __ENUMERATE_OPENTYPE_OPCODES(GC, 0x46)              \
    __ENUMERATE_OPENTYPE_OPCODES(GC_MAX, 0x47)          \
    __ENUMERATE_OPENTYPE_OPCODES(SCFS, 0x48)            \
    __ENUMERATE_OPENTYPE_OPCODES(MD, 0x49)              \
    __ENUMERATE_OPENTYPE_OPCODES(MD_MAX, 0x4A)          \
    __ENUMERATE_OPENTYPE_OPCODES(MPPEM, 0x4B)           \
    __ENUMERATE_OPENTYPE_OPCODES(MPS, 0x4C)             \
    /* Managing outlines */                             \
    __ENUMERATE_OPENTYPE_OPCODES(FLIPPT, 0x80)          \
    __ENUMERATE_OPENTYPE_OPCODES(FLIPRGON, 0x81)        \
    __ENUMERATE_OPENTYPE_OPCODES(FLIPRGOFF, 0x82)       \
    __ENUMERATE_OPENTYPE_OPCODES(SHP, 0x32)             \
    __ENUMERATE_OPENTYPE_OPCODES(SHP_MAX, 0x33)         \
    __ENUMERATE_OPENTYPE_OPCODES(SHC, 0x34)             \
    __ENUMERATE_OPENTYPE_OPCODES(SHC_MAX, 0x35)         \
    __ENUMERATE_OPENTYPE_OPCODES(SHZ, 0x36)             \
    __ENUMERATE_OPENTYPE_OPCODES(SHZ_MAX, 0x37)         \
    __ENUMERATE_OPENTYPE_OPCODES(SHPIX, 0x38)           \
    __ENUMERATE_OPENTYPE_OPCODES(MSIRP, 0x3A)           \
    __ENUMERATE_OPENTYPE_OPCODES(MSIRP_MAX, 0x3B)       \
    __ENUMERATE_OPENTYPE_OPCODES(MDAP, 0x2E)            \
    __ENUMERATE_OPENTYPE_OPCODES(MDAP_MAX, 0x2F)        \
    __ENUMERATE_OPENTYPE_OPCODES(MIAP, 0x3E)            \
    __ENUMERATE_OPENTYPE_OPCODES(MIAP_MAX, 0x3F)        \
    __ENUMERATE_OPENTYPE_OPCODES(MDRP, 0xC0)            \
    __ENUMERATE_OPENTYPE_OPCODES(MDRP_MAX, 0xDF)        \
    __ENUMERATE_OPENTYPE_OPCODES(MIRP, 0xE0)            \
    __ENUMERATE_OPENTYPE_OPCODES(MIRP_MAX, 0xFF)        \
    __ENUMERATE_OPENTYPE_OPCODES(ALIGNRP, 0x3C)         \
    __ENUMERATE_OPENTYPE_OPCODES(ISECT, 0x0F)           \
    __ENUMERATE_OPENTYPE_OPCODES(ALIGNPTS, 0x27)        \
    __ENUMERATE_OPENTYPE_OPCODES(IP, 0x39)              \
    __ENUMERATE_OPENTYPE_OPCODES(UTP, 0x29)             \
    __ENUMERATE_OPENTYPE_OPCODES(IUP, 0x30)             \
    __ENUMERATE_OPENTYPE_OPCODES(IUP_MAX, 0x31)         \
    /* Managing exceptions */                           \
    __ENUMERATE_OPENTYPE_OPCODES(DELTAP1, 0x5D)         \
    __ENUMERATE_OPENTYPE_OPCODES(DELTAP2, 0x71)         \
    __ENUMERATE_OPENTYPE_OPCODES(DELTAP3, 0x72)         \
    __ENUMERATE_OPENTYPE_OPCODES(DELTAC1, 0x73)         \
    __ENUMERATE_OPENTYPE_OPCODES(DELTAC2, 0x74)         \
    __ENUMERATE_OPENTYPE_OPCODES(DELTAC3, 0x75)         \
    /* Managing the stack */                            \
    __ENUMERATE_OPENTYPE_OPCODES(DUP, 0x20)             \
    __ENUMERATE_OPENTYPE_OPCODES(POP, 0x21)             \
    __ENUMERATE_OPENTYPE_OPCODES(CLEAR, 0x22)           \
    __ENUMERATE_OPENTYPE_OPCODES(SWAP, 0x23)            \
    __ENUMERATE_OPENTYPE_OPCODES(DEPTH, 0x24)           \
    __ENUMERATE_OPENTYPE_OPCODES(CINDEX, 0x25)          \
    __ENUMERATE_OPENTYPE_OPCODES(MINDEX, 0x26)          \
    __ENUMERATE_OPENTYPE_OPCODES(ROLL, 0x8a)            \
    /* Managing the flow of control */                  \
    __ENUMERATE_OPENTYPE_OPCODES(IF, 0x58)              \
    __ENUMERATE_OPENTYPE_OPCODES(ELSE, 0x1B)            \
    __ENUMERATE_OPENTYPE_OPCODES(EIF, 0x59)             \
    __ENUMERATE_OPENTYPE_OPCODES(JROT, 0x78)            \
    __ENUMERATE_OPENTYPE_OPCODES(JMPR, 0x1C)            \
    __ENUMERATE_OPENTYPE_OPCODES(JROF, 0x79)            \
    /* Logical functions */                             \
    __ENUMERATE_OPENTYPE_OPCODES(LT, 0x50)              \
    __ENUMERATE_OPENTYPE_OPCODES(LTEQ, 0x51)            \
    __ENUMERATE_OPENTYPE_OPCODES(GT, 0x52)              \
    __ENUMERATE_OPENTYPE_OPCODES(GTEQ, 0x53)            \
    __ENUMERATE_OPENTYPE_OPCODES(EQ, 0x54)              \
    __ENUMERATE_OPENTYPE_OPCODES(NEQ, 0x55)             \
    __ENUMERATE_OPENTYPE_OPCODES(ODD, 0x56)             \
    __ENUMERATE_OPENTYPE_OPCODES(EVEN, 0x57)            \
    __ENUMERATE_OPENTYPE_OPCODES(AND, 0x5A)             \
    __ENUMERATE_OPENTYPE_OPCODES(OR, 0x5B)              \
    __ENUMERATE_OPENTYPE_OPCODES(NOT, 0x5C)             \
    /* Arithmetic and math instructions */              \
    __ENUMERATE_OPENTYPE_OPCODES(ADD, 0x60)             \
    __ENUMERATE_OPENTYPE_OPCODES(SUB, 0x61)             \
    __ENUMERATE_OPENTYPE_OPCODES(DIV, 0x62)             \
    __ENUMERATE_OPENTYPE_OPCODES(MUL, 0x63)             \
    __ENUMERATE_OPENTYPE_OPCODES(ABS, 0x64)             \
    __ENUMERATE_OPENTYPE_OPCODES(NEG, 0x65)             \
    __ENUMERATE_OPENTYPE_OPCODES(FLOOR, 0x66)           \
    __ENUMERATE_OPENTYPE_OPCODES(CEILING, 0x67)         \
    __ENUMERATE_OPENTYPE_OPCODES(MAX, 0x8B)             \
    __ENUMERATE_OPENTYPE_OPCODES(MIN, 0x8C)             \
    /* Compensating for the engine characteristics */   \
    /* TODO: Are these opcodes? */                      \
    __ENUMERATE_OPENTYPE_OPCODES(ROUND, 0x68)           \
    __ENUMERATE_OPENTYPE_OPCODES(ROUND_MAX, 0x6B)       \
    __ENUMERATE_OPENTYPE_OPCODES(NROUND, 0x6C)          \
    __ENUMERATE_OPENTYPE_OPCODES(NROUND_MAX, 0x6F)      \
    /* Defining and using functions and instructions */ \
    __ENUMERATE_OPENTYPE_OPCODES(FDEF, 0x2C)            \
    __ENUMERATE_OPENTYPE_OPCODES(ENDF, 0x2D)            \
    __ENUMERATE_OPENTYPE_OPCODES(CALL, 0x2B)            \
    __ENUMERATE_OPENTYPE_OPCODES(LOOPCALL, 0x2A)        \
    __ENUMERATE_OPENTYPE_OPCODES(IDEF, 0x89)            \
    /* Debugging */                                     \
    __ENUMERATE_OPENTYPE_OPCODES(DEBUG, 0x4F)           \
    /* Miscellaneous instructions */                    \
    __ENUMERATE_OPENTYPE_OPCODES(GETINFO, 0x88)         \
    __ENUMERATE_OPENTYPE_OPCODES(GETVARIATION, 0x91)

// https://learn.microsoft.com/en-us/typography/opentype/spec/tt_instructions
enum class Opcode : u8 {
#define __ENUMERATE_OPENTYPE_OPCODES(opcode, value) opcode = value,
    ENUMERATE_OPENTYPE_OPCODES
#undef __ENUMERATE_OPENTYPE_OPCODES
};

StringView opcode_name(Opcode);

struct InstructionHandler;

struct InstructionStream {
    InstructionStream(InstructionHandler& handler, ReadonlyBytes bytes)
        : m_handler { handler }
        , m_bytes(bytes)
    {
    }

    void process_next_instruction();
    bool at_end() const;
    void jump_to_next(Opcode);

private:
    Opcode next_opcode() { static_cast<Opcode>(next_byte()); }

    u8 next_byte();
    ReadonlyBytes take_n_bytes(size_t n);

    InstructionHandler& m_handler;
    ReadonlyBytes m_bytes;
    size_t m_byte_index { 0 };
};

// Callbacks for each logically identical set of opcodes.
// Basic parsing handled by InstructionStream.
struct InstructionHandler {
    virtual void npush_bytes(InstructionStream&, ReadonlyBytes values) { }
    virtual void npush_words(InstructionStream&, ReadonlyBytes values) { }
    virtual void push_bytes(InstructionStream&, ReadonlyBytes values) { }
    virtual void push_words(InstructionStream&, ReadonlyBytes values) { }

    virtual void read_store(InstructionStream&) { }
    virtual void write_store(InstructionStream&) { }

    virtual void write_control_value_table_px(InstructionStream&) { }
    virtual void write_control_value_table_fuints(InstructionStream&) { }
    virtual void read_control_value_table(InstructionStream&) { }

    virtual void set_freedom_and_projection_vectors_to_coordinate_axis(InstructionStream&) { }
    virtual void set_projection_vector_to_coordinate_axis(InstructionStream&) { }
    virtual void set_freedom_vector_to_coordinate_axis(InstructionStream&) { }
    virtual void set_projection_vector_to_line(InstructionStream&) { }
    virtual void set_freedom_vector_vector_to_line(InstructionStream&) { }
    virtual void set_freedom_vector_to_projection_vector(InstructionStream&) { }
    virtual void set_dual_projection_vector_to_line(InstructionStream&) { }
    virtual void set_projection_vector_from_stack(InstructionStream&) { }
    virtual void set_freedom_vector_from_stack(InstructionStream&) { }
    virtual void get_projection_vector(InstructionStream&) { }
    virtual void get_freedom_vector(InstructionStream&) { }
    virtual void set_refrence_point(InstructionStream&, int reference_point) { }
    virtual void set_zone_pointer(InstructionStream&, int zone_pointer) { }
    virtual void set_zone_pointers(InstructionStream&) { }
    virtual void round_to_half_grid(InstructionStream&) { }
    virtual void round_to_grid(InstructionStream&) { }
    virtual void round_to_double_grid(InstructionStream&) { }
    virtual void round_down_to_grid(InstructionStream&) { }
    virtual void round_up_to_grid(InstructionStream&) { }
    virtual void round_off(InstructionStream&) { }
    virtual void super_round(InstructionStream&) { }
    virtual void super_round_45_degrees(InstructionStream&) { }
    virtual void set_loop_variable(InstructionStream&) { }
    virtual void set_minimum_distance(InstructionStream&) { }
    virtual void instruction_execution_control(InstructionStream&) { }
    virtual void scan_conversion_control(InstructionStream&) { }
    virtual void scantype(InstructionStream&) { }
    virtual void set_control_value_table_in(InstructionStream&) { }
    virtual void set_single_width_cut_in(InstructionStream&) { }
    virtual void set_single_width(InstructionStream&) { }
    virtual void set_auto_flip_boolean(InstructionStream&, bool) { }
    virtual void set_angle_weight(InstructionStream&) { }
    virtual void set_delta_base(InstructionStream&) { }
    virtual void set_delta_shift(InstructionStream&) { }

    virtual void get_coordinate_projected_onto_projection_vector(InstructionStream&, bool a) { }
    virtual void set_coordinate_from_stack_using_projection_and_freedom_vectors(InstructionStream&) { }
    virtual void measure_distance(InstructionStream&, bool a) { }
    virtual void measure_pixels_per_em(InstructionStream&) { }
    virtual void measure_point_size(InstructionStream&) { }
    virtual void flip_point(InstructionStream&) { }
    virtual void flip_range_on(InstructionStream&) { }
    virtual void flip_range_off(InstructionStream&) { }
    virtual void shift_point_by_last_point(InstructionStream&, bool a) { }
    virtual void shift_contour_by_last_point(InstructionStream&, bool a) { }
    virtual void shift_zone_by_last_point(InstructionStream&, bool a) { }
    virtual void shift_point_by_pixel_amount(InstructionStream&) { }
    virtual void move_stack_indirect_relative_point(InstructionStream&, bool a) { }
    virtual void move_direct_absolute_point(InstructionStream&, bool a) { }
    virtual void move_indirect_absolute_point(InstructionStream&, bool a) { }
    virtual void move_direct_relative_point(InstructionStream&, bool a, bool b, bool c) { }
    virtual void move_indirect_relative_point(InstructionStream&, bool a, bool b, bool c) { }
    virtual void align_relative_point(InstructionStream&) { }
    virtual void adjust_angle(InstructionStream&) { }
    virtual void align_points(InstructionStream&) { }
    virtual void interpolate_point_by_last_relative_stretch(InstructionStream&) { }
    virtual void untouch_point(InstructionStream&) { }
    virtual void interpolate_untouched_points_through_outline(InstructionStream&) { }

    virtual void delta_exception_p(InstructionStream&, int p) { }
    virtual void delta_exception_c(InstructionStream&, int c) { }

    virtual void stack_dup(InstructionStream&) { }
    virtual void stack_clear(InstructionStream&) { }
    virtual void stack_swap(InstructionStream&) { }
    virtual void stack_depth(InstructionStream&) { }
    virtual void stack_copy_indexed_element(InstructionStream&) { }
    virtual void stack_move_indexed_element(InstructionStream&) { }
    virtual void stack_roll_three_elements(InstructionStream&) { }

    virtual void if_test(InstructionStream&) { }
    virtual void else_case(InstructionStream&) { }
    virtual void end_if(InstructionStream&) { }
    virtual void jump_relative_on_true(InstructionStream&) { }
    virtual void jump(InstructionStream&) { }
    virtual void jump_relative_on_flase(InstructionStream&) { }

    virtual void less_than(InstructionStream&) { }
    virtual void less_than_or_equal(InstructionStream&) { }
    virtual void greater_than(InstructionStream&) { }
    virtual void greater_than_or_equal(InstructionStream&) { }
    virtual void equal(InstructionStream&) { }
    virtual void not_equal(InstructionStream&) { }
    virtual void odd(InstructionStream&) { }
    virtual void even(InstructionStream&) { }
    virtual void logical_and(InstructionStream&) { }
    virtual void logical_or(InstructionStream&) { }
    virtual void logical_not(InstructionStream&) { }

    virtual void add(InstructionStream&) { }
    virtual void subtract(InstructionStream&) { }
    virtual void divide(InstructionStream&) { }
    virtual void multiply(InstructionStream&) { }
    virtual void absolute_value(InstructionStream&) { }
    virtual void negate(InstructionStream&) { }
    virtual void floor(InstructionStream&) { }
    virtual void ceiling(InstructionStream&) { }
    virtual void max(InstructionStream&) { }
    virtual void min(InstructionStream&) { }

    virtual void round(InstructionStream&) { }
    virtual void no_round(InstructionStream&) { }

    virtual void function_definition(InstructionStream&) { }
    virtual void end_function_definition(InstructionStream&) { }
    virtual void call_function(InstructionStream&) { }
    virtual void loop_and_call_function(InstructionStream&) { }
    virtual void instruction_definition(InstructionStream&) { }

    virtual void debug_call(InstructionStream&) { }

    virtual void get_information(InstructionStream&) { }
    virtual void get_variation(InstructionStream&) { }
};

struct Interpreter : InstructionHandler {

    // TODO:
    struct Zone { };

    // https://learn.microsoft.com/en-us/typography/opentype/spec/tt_graphics_state
    struct GraphicsState {
        bool auto_flip { true };
        F26Dot6 control_value_cut_in { 17 };
        u32 delta_base { 9 };
        u32 delta_shift { 3 };
        Gfx::Point<F2Dot14> dual_projection_vectors;
        Gfx::Point<F2Dot14> freedom_vector;
        Zone* zp0 { nullptr };
        Zone* zp1 { nullptr };
        Zone* zp2 { nullptr };
        u32 loop { 0 };
        F26Dot6 minimum_distance { 1 };
        Gfx::Point<F2Dot14> projection_vector;
        u8 round_state { 1 };
        u32 rp0 { 0 };
        u32 rp1 { 0 };
        u32 rp2 { 0 };
        bool scan_control { false };
        F26Dot6 singe_width_cut_in { 0 };
        F26Dot6 single_width_value { 0 };
    };
};

}
