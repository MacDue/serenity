/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Font/OpenType/Interpreter.h>

namespace OpenType {

StringView Interpreter::opcode_name(Opcode opcode)
{
    switch (opcode) {
    case NPUSHB:
        return "NPUSHB";
        NPUSHW = 0x41,
        PUSHB = 0xB0,
        PUSHB_MAX = 0xB7,
        PUSHW = 0xB8,
        PUSHW_MAX = 0xBF,
        // Managing the Storage Area
            RS = 0x43,
        WS = 0x42,
        // Managing the Control Value Table
            WCVTP = 0x44,
        WCVTF = 0x70,
        RCVT = 0x45,
        // Managing the Graphics State
            SVTCA = 0x00,
        SVTCA_MAX = 0x1,
        SPVTCA = 0x02,
        SPVTCA_MAX = 0x3,
        SFVTCA = 0x04,
        SFVTCA_MAX = 0x05,
        SPVTL = 0x06,
        SPVTL_MAX = 0x07,
        SFVTL = 0x08,
        SFVTL_MAX = 0x09,
        SFVTPV = 0x0E,
        SDPVTL = 0x86,
        SDPVTL_MAX = 0x87,
        SPVFS = 0x0A,
        SFVFS = 0x0B,
        GPV = 0x0C,
        GFV = 0x0D,
        SRP0 = 0x10,
        SRP1 = 0x11,
        SRP2 = 0x12,
        SZP0 = 0x13,
        SZP1 = 0x14,
        SZP2 = 0x15,
        SZPS = 0x16,
        RTHG = 0x19,
        RTG = 0x18,
        RTDG = 0x3D,
        RDTG = 0x7D,
        RUTG = 0x7C,
        ROFF = 0x7A,
        SROUND = 0x76,
        S45ROUND = 0x77,
        SLOOP = 0x17,
        SMD = 0x1A,
        INSTCTRL = 0x8E,
        SCANCTRL = 0x85,
        SCANTYPE = 0x8D,
        SCVTCI = 0x1D,
        SSWCI = 0x1E,
        SSW = 0x1F,
        FLIPON = 0x4D,
        FLIPOFF = 0x4E,
        SANGW = 0x7E,
        SDB = 0x5E,
        SDS = 0x5F,
        // Reading and writing data
            GC = 0x46,
        GC_MAX = 0x47,
        SCFS = 0x48,
        MD = 0x49,
        MD_MAX = 0x4A,
        MPPEM = 0x4B,
        MPS = 0x4C,
        // Managing outlines
            FLIPPT = 0x80,
        FLIPRGON = 0x81,
        FLIPRGOFF = 0x82,
        SHP = 0x32,
        SHP_MAX = 0x33,
        SHC = 0x34,
        SHC_MAX = 0x35,
        SHZ = 0x36,
        SHZ_MAX = 0x37,
        SHPIX = 0x38,
        MSIRP = 0x3A,
        MSIRP_MAX = 0x3B,
        MDAP = 0x2E,
        MDAP_MAX = 0x2F,
        MIAP = 0x3E,
        MIAP_MAX = 0x3F,
        MDRP = 0xC0,
        MDRP_MAX = 0xDF,
        MIRP = 0xE0,
        MIRP_MAX = 0xFF,
        ALIGNRP = 0x3C,
        ISECT = 0x0F,
        ALIGNPTS = 0x27,
        IP = 0x39,
        UTP = 0x29,
        IUP = 0x30,
        IUP_MAX = 0x31,
        // Managing exceptions
            DELTAP1 = 0x5D,
        DELTAP2 = 0x71,
        DELTAP3 = 0x72,
        DELTAC1 = 0x73,
        DELTAC2 = 0x74,
        DELTAC3 = 0x75,
        // Managing the stack
            DUP = 0x20,
        POP = 0x21,
        CLEAR = 0x22,
        SWAP = 0x23,
        DEPTH = 0x24,
        CINDEX = 0x25,
        MINDEX = 0x26,
        ROLL = 0x8a,
        // Managing the flow of control
            IF = 0x58,
        ELSE = 0x1B,
        EIF = 0x59,
        JROT = 0x78,
        JMPR = 0x1C,
        JROF = 0x79,
        // Logical functions
            LT = 0x50,
        LTEQ = 0x51,
        GT = 0x52,
        GTEQ = 0x53,
        EQ = 0x54,
        NEQ = 0x55,
        ODD = 0x56,
        EVEN = 0x57,
        AND = 0x5A,
        OR = 0x5B,
        NOT = 0x5C,
        // Arithmetic and math instructions
            ADD = 0x60,
        SUB = 0x61,
        DIV = 0x62,
        MUL = 0x63,
        ABS = 0x64,
        NEG = 0x65,
        FLOOR = 0x66,
        CEILING = 0x67,
        MAX = 0x8B,
        MIN = 0x8C,
        // Compensating for the engine characteristics
        // TODO: Are these opcodes?
            ROUND = 0x68,
        ROUND_MAX = 0x6B,
        NROUND = 0x6C,
        NROUND_MAX = 0x6F,
        // Defining and using functions and instructions
            FDEF = 0x2C,
        ENDF = 0x2D,
        CALL = 0x2B,
        LOOPCALL = 0x2A,
        IDEF = 0x89,
        // Debugging
            DEBUG = 0x4F,
        // Miscellaneous instructions
            GETINFO = 0x88,
        GETVARIATION = 0x91
    }
}

void Interpreter::debug_print_instruction_stream(ReadonlyBytes stream)
{
    auto it = stream.begin();

    auto next_opcode = [&]() -> Opcode {
        return static_cast<Opcode>(*(it++));
    };

    auto next_byte = [&]() -> u8 {
        return *it++;
    };

    auto next_word = [&]() -> u16 {
        return 0;
    };

    while (it != stream.end()) {
        auto opcode = next_opcode();
    }
}

}
