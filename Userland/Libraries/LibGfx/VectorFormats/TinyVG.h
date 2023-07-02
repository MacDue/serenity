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

// Current recommended SVG to TVG conversion (without installing tools)
// (FIXME: Implement our own converter!)
// 1. (Optional) Convert strokes to fills
//  * Strokes are not well represented in TVG, converting them to fills
//    (that still beziers etc) works much better.
//  * This site can do that: https://iconly.io/tools/svg-convert-stroke-to-fill
// 2. Scale your SVG's width/height to large size (e.g. 1024x?)
//  * Current converters deal very poorly with small values in paths.
//  * This site can do that: https://www.iloveimg.com/resize-image/resize-svg
//    (or just edit the viewbox if it has one).
// 3. Convert the SVG to a TVG
//  * This site can do that: https://svg-to-tvg-server.fly.dev/

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

    IntSize size() const
    {
        return m_size;
    }

    ReadonlySpan<DrawCommand> draw_commands() const
    {
        return m_draw_commands;
    }

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
