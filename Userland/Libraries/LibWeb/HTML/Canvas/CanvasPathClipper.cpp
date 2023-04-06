/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/AntiAliasingPainter.h>
#include <LibWeb/HTML/Canvas/CanvasPathClipper.h>

namespace Web::HTML {

// FIXME: This pretty naive, we should be able to cut down the allocations here
// (especially for the paint style which is a bit sad).

ErrorOr<CanvasPathClipper> CanvasPathClipper::create(Gfx::Painter& painter, CanvasClip const& canvas_clip)
{
    auto bounding_box = enclosing_int_rect(canvas_clip.path.bounding_box());
    Gfx::IntRect actual_save_rect {};
    auto saved_bitmap = TRY(painter.get_region_bitmap(bounding_box, Gfx::BitmapFormat::BGRA8888, actual_save_rect));
    painter.save();
    painter.add_clip_rect(bounding_box);
    return CanvasPathClipper(*saved_bitmap, actual_save_rect.location(), canvas_clip);
}

ErrorOr<void> CanvasPathClipper::apply_clip(Gfx::Painter& painter)
{
    painter.restore();
    auto bounding_box = enclosing_int_rect(m_canvas_clip.path.bounding_box());
    Gfx::IntRect actual_save_rect {};
    auto clip_area = TRY(painter.get_region_bitmap(bounding_box, Gfx::BitmapFormat::BGRA8888, actual_save_rect));
    painter.blit(m_save_location, *m_saved_bitmap, m_saved_bitmap->rect(), 1.0f, false);
    Gfx::AntiAliasingPainter aa_painter { painter };
    aa_painter.fill_path(m_canvas_clip.path, TRY(Gfx::BitmapPaintStyle::create(*clip_area)), m_canvas_clip.winding_rule);
    return {};
}
}
