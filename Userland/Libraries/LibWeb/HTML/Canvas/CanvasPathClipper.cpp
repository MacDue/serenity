/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/AntiAliasingPainter.h>
#include <LibGfx/Path.h>
#include <LibWeb/HTML/Canvas/CanvasPathClipper.h>

namespace Web::HTML {
class BitmapPaintStyle : public Gfx::PaintStyle {
public:
    BitmapPaintStyle(Gfx::Bitmap& bitmap)
        : m_bitmap(bitmap)
    {
    }

    virtual Color sample_color(Gfx::IntPoint point) const
    {
        if (m_bitmap->rect().contains(point))
            return m_bitmap->get_pixel(point);
    }

private:
    NonnullRefPtr<Gfx::Bitmap const> m_bitmap;
};

ErrorOr<CanvasPathClipper> CanvasPathClipper::create(Gfx::Painter& painter, CanvasClip const& canvas_clip)
{
    auto bounding_box = enclosing_int_rect(canvas_clip.path.bounding_box());
    Gfx::IntRect actual_save_rect {};
    auto saved_bitmap = TRY(painter.get_region_bitmap(bounding_box, Gfx::BitmapFormat::BGRA8888, actual_save_rect));
    return CanvasPathClipper(*saved_bitmap, actual_save_rect.location(), canvas_clip);
}

ErrorOr<void> CanvasPathClipper::apply_clip(Gfx::Painter& painter)
{
    auto bounding_box = enclosing_int_rect(m_canvas_clip.path.bounding_box());
    Gfx::IntRect actual_save_rect {};
    auto clip_area = TRY(painter.get_region_bitmap(bounding_box, Gfx::BitmapFormat::BGRA8888, actual_save_rect));
    painter.blit(m_save_location, *m_saved_bitmap, m_saved_bitmap->rect(), 1.0f, false);
    Gfx::AntiAliasingPainter aa_painter { painter };
    BitmapPaintStyle clip_area_style(*clip_area);
    aa_painter.fill_path(m_canvas_clip.path, clip_area_style, m_canvas_clip.winding_rule);
}
}
