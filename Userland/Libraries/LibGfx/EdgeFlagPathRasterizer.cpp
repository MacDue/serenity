/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Array.h>
#include <AK/Types.h>
#include <LibGfx/EdgeFlagPathRasterizer.h>
#include <LibGfx/Painter.h>

// This a very naive implementation of edge-flag scanline AA.
// The paper lists many possible optimizations, maybe implement one? (FIXME!)
// https://mlab.taik.fi/~kkallio/antialiasing/EdgeFlagAA.pdf

namespace Gfx {

EdgeFlagPathRasterizer::EdgeFlagPathRasterizer(Gfx::IntSize size)
    : m_size(size.width() + 10, size.height() + 10)
{
    m_data.resize(m_size.width() * m_size.height());
}

RefPtr<Gfx::Bitmap> EdgeFlagPathRasterizer::accumulate()
{
    // for (int y = 0; y < m_data->height(); y++) {
    //     auto* line = m_data->scanline(y);
    //     u8 pen_alpha = 0;
    //     for (int x = 0; x < m_data->width(); x++) {
    //         pen_alpha ^= Color::from_argb(line[x]).alpha();
    //         line[x] = Color(Color::Black).with_alpha(pen_alpha).value();
    //     }
    // }
    auto result = MUST(Gfx::Bitmap::create(BitmapFormat::BGRA8888, m_size));
    for (int y = 0; y < m_size.height(); y += 1) {
        u8 sample = 0;
        for (int x = 0; x < m_size.width(); x += 1) {
            sample ^= m_data[y * m_size.width() + x];
            auto coverage = popcount(sample);
            auto alpha = clamp(coverage << 5, 0, 255);
            result->set_pixel(x, y, Color(Color::Black).with_alpha(alpha));
        }
    }
    return result;
}

void EdgeFlagPathRasterizer::draw_path(Gfx::Path& path)
{
    for (auto& line : path.split_lines())
        draw_line(line.from, line.to);
}

void EdgeFlagPathRasterizer::draw_line(Gfx::FloatPoint p0, Gfx::FloatPoint p1)
{
    p0.translate_by(5, 5);
    p1.translate_by(5, 5);

    // if (p0.x() < 0.f || p0.y() < 0.f || p0.x() > m_size.width() || p0.y() > m_size.height()) {
    //     dbgln("!P0({},{})", p0.x(), p0.y());
    //     return;
    // }

    // if (p1.x() < 0.f || p1.y() < 0.f || p1.x() > m_size.width() || p1.y() > m_size.height()) {
    //     dbgln("!P1({},{})", p1.x(), p1.y());
    //     return;
    // }

    // p0 = Gfx::FloatPoint(m_size.width() / 2, 0);
    // p1 = Gfx::FloatPoint(m_size.width() / 2, m_size.height());

    p0.scale_by(1, 8);
    p1.scale_by(1, 8);

    Array offsets { 0.25f, 0.875f, 0.5f, 0.125f, 0.75f, 0.375f, 0.0f, 0.625f };
    if (p0.y() > p1.y())
        swap(p0, p1);

    auto dx = p1.x() - p0.x();
    auto dy = p1.y() - p0.y();
    if (dy == 0)
        return;

    float dxdy = float(dx) / dy;

    // dbgln("foo {}", dxdy);

    float x = p0.x();
    for (int y = p0.y(); y < p1.y(); y++) {
        int y_sub = y % 8;
        int y_bit = y / 8;
        int xi = (int)(x + offsets[y_sub]);
        u8 sample = 1 << y_sub;
        int idx = y_bit * m_size.width() + xi;
        // if (idx < (int)m_data.size()) {
        m_data[idx] |= sample;
        // if (idx == 3569)
        //   dbgln("{:04} - {:08b} {:08b} {:08b}", idx, data, sample, m_data[idx]);
        // }
        x += dxdy;
    }
}

}
