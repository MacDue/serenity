/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/FixedPoint.h>
#include <AK/Span.h>
#include <AK/Vector.h>
#include <LibGfx/Font/Font.h>
#include <LibGfx/Font/OpenType/Font.h>
#include <LibGfx/Font/OpenType/Hinting/Opcodes.h>
#include <LibGfx/Point.h>

namespace OpenType::Hinting {

using F2Dot14 = FixedPoint<30, u16>;
using F2Dot30 = FixedPoint<30, u32>;
using F26Dot6 = FixedPoint<6, u32>;

struct Interpreter : InstructionHandler {

    static ErrorOr<Interpreter> create(Font& font);

private:
    // TODO:
    struct Zone { };
    struct Curves { };

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

    struct Stack {
        Stack(size_t max_stack_depth);

        uint32_t pop();
        void push(u32);
        void push_byte(u8);
        void push_word(u16);

    private:
        Vector<u32> m_stack;
    };

    struct HintingData {
        Curves curves;
        Zone zone1;
        Stack stack;
        Vector<ReadonlyBytes> functions;
        GraphicsState graphics_state;
    };

    HintingData m_hinting_data;
};

}
