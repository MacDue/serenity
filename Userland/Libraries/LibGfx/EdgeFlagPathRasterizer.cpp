/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/EdgeFlagPathRasterizer.h>
#include <LibGfx/Painter.h>

namespace Gfx {

EdgeFlagPathRasterizer::EdgeFlagPathRasterizer(Gfx::IntSize size)
    : m_size(size)
{
    m_data = MUST(Gfx::Bitmap::create(BitmapFormat::BGRA8888, size.scaled_by(8, 8)));
}

RefPtr<Gfx::Bitmap> EdgeFlagPathRasterizer::accumulate()
{
    for (int y = 0; y < m_data->height(); y++) {
        auto* line = m_data->scanline(y);
        u8 pen_alpha = 0;
        for (int x = 0; x < m_data->width(); x++) {
            pen_alpha ^= Color::from_argb(line[x]).alpha();
            line[x] = Color(Color::Black).with_alpha(pen_alpha).value();
        }
    }
    auto result = MUST(Gfx::Bitmap::create(BitmapFormat::BGRA8888, m_size));
    for (int y = 0; y < m_data->height(); y += 8) {
        for (int x = 0; x < m_data->width(); x += 8) {
            u32 area = 0;
            for (int sy = y; sy < y + 8; sy++) {
                for (int sx = x; sx < x + 8; sx++) {
                    area += m_data->get_pixel(sx, sy).alpha();
                }
            }
            result->set_pixel(x / 8, y / 8, Color(Color::Black).with_alpha(area / (8 * 8)));
        }
    }
    return result;
}

void EdgeFlagPathRasterizer::draw_path(Gfx::Path& path)
{
    Gfx::Painter painter(*m_data);
    for (auto& line : path.split_lines())
        draw_line(line.from, line.to);
}

void EdgeFlagPathRasterizer::draw_line(Gfx::FloatPoint p0, Gfx::FloatPoint p1)
{
    if (p0.x() < 0.f || p0.y() < 0.f || p0.x() > m_size.width() || p0.y() > m_size.height()) {
        dbgln("!P0({},{})", p0.x(), p0.y());
        return;
    }

    if (p1.x() < 0.f || p1.y() < 0.f || p1.x() > m_size.width() || p1.y() > m_size.height()) {
        dbgln("!P1({},{})", p1.x(), p1.y());
        return;
    }

    auto int_p0 = p0.scaled(8, 8).to_type<int>();
    auto int_p1 = p1.scaled(8, 8).to_type<int>();

    if (int_p0.y() > int_p1.y())
        swap(int_p0, int_p1);

    int const dx = int_p1.x() - int_p0.x();
    int const dy = int_p1.y() - int_p0.y();
    float dxdy = float(dx) / dy;

    float x = int_p0.x();
    for (int y = int_p0.y(); y < int_p1.y(); ++y) {
        if (x < m_data->width())
            m_data->set_pixel(x, y, Color::Black);
        x += dxdy;
    }
}

}
