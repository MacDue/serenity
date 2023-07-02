/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Forward.h>
#include <AK/Vector.h>
#include <LibGfx/Color.h>
#include <LibGfx/Forward.h>
#include <LibGfx/PaintStyle.h>
#include <LibGfx/Path.h>

namespace Gfx {

class TinyVG {
public:
    using Style = Variant<Color, NonnullRefPtr<SVGGradientPaintStyle>>;

    struct DrawCommand {
        Path path;
        Optional<Style> fill {};
        Optional<Style> stroke {};
        float stroke_width { 0.0f };
    };

    static ErrorOr<TinyVG> decode(Stream& stream);
    static ErrorOr<TinyVG> read_from_file(StringView path);

    ErrorOr<RefPtr<Gfx::Bitmap>> bitmap(IntSize size) const;

private:
    TinyVG(IntSize size, Vector<DrawCommand> draw_commands)
        : m_size(size)
        , m_draw_commands(move(draw_commands))
    {
    }

    IntSize m_size;
    Vector<DrawCommand> m_draw_commands;
};

}
