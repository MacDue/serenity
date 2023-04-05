/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibGfx/Painter.h>

namespace Web::HTML {

struct CanvasClip {
    Gfx::Path path;
    Gfx::Painter::WindingRule winding_rule;
};

class CanvasPathClipper {
public:
    static ErrorOr<CanvasPathClipper> create(Gfx::Painter&, CanvasClip const& canvas_clip);

    ErrorOr<void> apply_clip(Gfx::Painter& painter);

private:
    CanvasPathClipper(Gfx::Bitmap const& saved_bitmap, Gfx::IntPoint save_location, CanvasClip const& canvas_clip)
        : m_saved_bitmap(saved_bitmap)
        , m_save_location(save_location)
        , m_canvas_clip(canvas_clip)
    {
    }

    NonnullRefPtr<Gfx::Bitmap const> m_saved_bitmap;
    Gfx::IntPoint m_save_location;
    CanvasClip const& m_canvas_clip;
};

struct ScopedCanvasPathClip {
    ScopedCanvasPathClip(Gfx::Painter& painter, Optional<CanvasClip> const& canvas_clip)
        : m_painter(painter)
    {
        if (canvas_clip.has_value()) {
            auto clipper = CanvasPathClipper::create(painter, *canvas_clip);
            if (!clipper.is_error())
                m_canvas_clipper = clipper.release_value();
        }
    }

    ~ScopedCanvasPathClip()
    {
        if (m_canvas_clipper.has_value())
            m_canvas_clipper->apply_clip(m_painter);
    }

    AK_MAKE_NONMOVABLE(ScopedCanvasPathClip);
    AK_MAKE_NONCOPYABLE(ScopedCanvasPathClip);

private:
    Gfx::Painter& m_painter;
    Optional<CanvasPathClipper> m_canvas_clipper;
};

}
