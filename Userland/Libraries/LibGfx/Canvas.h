/*
 * Copyright (c) 2024, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/HashMap.h>
#include <LibGfx/AffineTransform.h>
#include <LibGfx/AntiAliasingPainter.h>
#include <LibGfx/Painter.h>
#include <LibGfx/Path.h>
#include <LibGfx/Rect.h>
#include <LibGfx/TextLayout.h>

namespace Gfx {

class Canvas {
public:
    void draw_glyphs(ReadonlySpan<DrawGlyphOrEmoji> glyphs, Color color, Gfx::FloatPoint translation, float scale);

    void fill_rect(FloatRect const&, Color);

    void fill_rect(FloatRect const&, PaintStyle const&, float opacity = 1.0f);

    void draw_line(FloatLine const&, Color, float thickness);

    void draw_rect(FloatRect const&, Color, float thickness);

    void fill_path(Path const&, Color, Painter::WindingRule = Painter::WindingRule::Nonzero);

    void fill_path(Path const&, PaintStyle const&, float opacity = 1.0f, Painter::WindingRule = Painter::WindingRule::Nonzero);

    void stroke_path(Path const&, Color, float thickness);

    void stroke_path(Path const&, PaintStyle const&, float thickness, float opacity = 1.0f);

    void draw_bitmap(FloatRect const& dst_rect, Bitmap const&, FloatRect const& src_rect, float opacity = 1.0f, Painter::ScalingMode = Painter::ScalingMode::BilinearBlend);

    void apply_transform(Gfx::AffineTransform const& transform)
    {
        layer().transform.multiply(transform);
    }

    void set_clip(FloatRect const& rect);

    void set_clip(Path const& path);

    void clear_clip()
    {
        layer().clip = {};
        layer().painter.clear_clip_rect();
    }

    void push_layer(float opacity);

    void pop_layer();

    void flatten();

    Canvas(Bitmap& target);

private:
    void apply_current_clip();

    Gfx::AntiAliasingPainter painter()
    {
        return Gfx::AntiAliasingPainter { layer().painter };
    }

    struct Clip {
        RefPtr<Bitmap> mask = nullptr;
    };

    NonnullRefPtr<Bitmap> new_bitmap() const;

    struct Layer {
        Clip clip;
        float opacity = 1.0f;
        AffineTransform transform;
        RefPtr<Bitmap> target;
        // FIXME: Get rid of old painter usage.
        Painter painter;
    };

    Layer& layer() { return m_layer_stack.last(); }
    Layer const& layer() const { return m_layer_stack.last(); }

    Vector<Layer> m_layer_stack;
};

}
