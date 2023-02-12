/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/FixedArray.h>
#include <AK/FixedPoint.h>
#include <AK/NonnullOwnPtr.h>
#include <AK/Span.h>
#include <LibGfx/Font/Font.h>
#include <LibGfx/Font/OpenType/Font.h>
#include <LibGfx/Font/OpenType/Hinting/Opcodes.h>
#include <LibGfx/Point.h>

namespace OpenType::Hinting {

using F2Dot14 = FixedPoint<30, u16>;
using F2Dot30 = FixedPoint<30, u32>;
using F26Dot6 = FixedPoint<6, u32>;

class Interpreter final : public InstructionHandler
    , public RefCounted<Interpreter> {
public:
    // TODO: Figure out what data this needs to mutate
    static ErrorOr<NonnullRefPtr<Interpreter>> create(Font& font);

    // TODO:
    struct Zone { };

    // A font at a specfic size.
    struct FontInstanceData {
        u32 ppem;
        FixedArray<u32> storage_area;
        FixedArray<F26Dot6> cvt;
        Zone zone0;
    };
    ErrorOr<NonnullOwnPtr<FontInstanceData>> create_font_instance_data(u32 ppem);

    struct ProgramContext {
        Optional<FontInstanceData&> instance {};
    };

    void execute_program(InstructionStream, ProgramContext);

private:
    virtual void default_handler(Context) override;

    void delta_c();

    virtual void handle_NPUSHB(Context) override;
    virtual void handle_NPUSHW(Context) override;
    virtual void handle_PUSHB(Context) override;
    virtual void handle_PUSHW(Context) override;
    virtual void handle_FDEF(Context) override;
    virtual void handle_CALL(Context) override;
    virtual void handle_SVTCA(Context) override;
    virtual void handle_MPPEM(Context) override;
    virtual void handle_EQ(Context) override;
    virtual void handle_WS(Context) override;
    virtual void handle_GETINFO(Context) override;
    virtual void handle_LTEQ(Context) override;
    virtual void handle_GTEQ(Context) override;
    virtual void handle_AND(Context) override;
    virtual void handle_IF(Context) override;
    virtual void handle_RS(Context) override;
    virtual void handle_ADD(Context) override;
    virtual void handle_MUL(Context) override;
    virtual void handle_GT(Context) override;
    virtual void handle_LT(Context) override;
    virtual void handle_OR(Context) override;
    virtual void handle_NEQ(Context) override;
    virtual void handle_SCANCTRL(Context) override;
    virtual void handle_SCANTYPE(Context) override;
    virtual void handle_SCVTCI(Context) override;
    virtual void handle_DELTAC1(Context) override;
    virtual void handle_DELTAC2(Context) override;
    virtual void handle_DELTAC3(Context) override;
    virtual void handle_POP(Context) override;
    virtual void handle_RCVT(Context) override;
    virtual void handle_RTG(Context) override;
    virtual void handle_ROUND(Context) override;
    virtual void handle_SWAP(Context) override;
    virtual void handle_DUP(Context) override;
    virtual void handle_ABS(Context) override;
    virtual void handle_RUTG(Context) override;
    virtual void handle_ROLL(Context) override;
    virtual void handle_WCVTP(Context) override;
    virtual void handle_SDB(Context) override;

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
        F2Dot30 projection_dot_free;
        Gfx::Point<F2Dot14> projection_vector;
        u8 round_state { 1 };
        u8 scan_type { 0 };
        u32 rp0 { 0 };
        u32 rp1 { 0 };
        u32 rp2 { 0 };
        bool scan_control { false };
        F26Dot6 singe_width_cut_in { 0 };
        F26Dot6 single_width_value { 0 };
    };

    struct Stack {
        Stack(FixedArray<u32> stack)
            : m_stack(move(stack))
        {
        }

        void clear() { m_top = 0; }
        uint32_t pop();
        void push(u32);
        void push_byte(u8);
        void push_word(i16);

    private:
        size_t m_top { 0 };
        FixedArray<u32> m_stack;
    };

    struct HintingData {
        Curves curves;
        Zone zone1;
        Stack stack;
        FixedArray<ReadonlyBytes> functions;
        GraphicsState graphics_state;
    };

    Interpreter(Font& font, HintingData hinting_data)
        : m_font(font)
        , m_hinting_data(move(hinting_data))
    {
    }

    NonnullRefPtr<Font> m_font;
    HintingData m_hinting_data;
    ProgramContext m_context;
};

}
