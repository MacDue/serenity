/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/FixedPoint.h>
#include <AK/Span.h>
#include <LibGfx/Point.h>

namespace OpenType {

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

struct Interpreter {

    // https://learn.microsoft.com/en-us/typography/opentype/spec/tt_instructions
    enum class Opcode : u8 {
#define __ENUMERATE_OPENTYPE_OPCODES(opcode, value) opcode = value,
        ENUMERATE_OPENTYPE_OPCODES
#undef __ENUMERATE_OPENTYPE_OPCODES
    };

    static StringView opcode_name(Opcode);
    static void debug_print_instruction_stream(ReadonlyBytes stream);

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
