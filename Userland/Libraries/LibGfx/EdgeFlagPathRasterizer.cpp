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
    : m_size(size.width() + 1, size.height())
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
        u32 sample = 0;
        for (int x = 0; x < m_size.width(); x += 1) {
            sample ^= m_data[y * m_size.width() + x];
            auto coverage = popcount(sample);
            auto alpha = clamp(coverage << 3, 0, 255);
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
    // p0.translate_by(5, 5);
    // p1.translate_by(5, 5);

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

    p0.scale_by(1, 32);
    p1.scale_by(1, 32);

    Array offsets {
        (28.0f / 32.0f),
        (13.0f / 32.0f),
        (6.0f / 32.0f),
        (23.0f / 32.0f),
        (0.0f / 32.0f),
        (17.0f / 32.0f),
        (10.0f / 32.0f),
        (27.0f / 32.0f),
        (4.0f / 32.0f),
        (21.0f / 32.0f),
        (14.0f / 32.0f),
        (31.0f / 32.0f),
        (8.0f / 32.0f),
        (25.0f / 32.0f),
        (18.0f / 32.0f),
        (3.0f / 32.0f),
        (12.0f / 32.0f),
        (29.0f / 32.0f),
        (22.0f / 32.0f),
        (7.0f / 32.0f),
        (16.0f / 32.0f),
        (1.0f / 32.0f),
        (26.0f / 32.0f),
        (11.0f / 32.0f),
        (20.0f / 32.0f),
        (5.0f / 32.0f),
        (30.0f / 32.0f),
        (15.0f / 32.0f),
        (24.0f / 32.0f),
        (9.0f / 32.0f),
        (2.0f / 32.0f),
        (19.0f / 32.0f),
    };
    if (p0.y() > p1.y())
        swap(p0, p1);

    if (p0.y() == p1.y())
        return;

    // p1.translate_by(0, 10);

    auto dx = p1.x() - p0.x();
    auto dy = p1.y() - p0.y();

    double dxdy = double(dx) / dy;

    // if (dx >= dy)
    // return;

    // if (p0.y() > 70)
    //   return;

    // dbgln("{}", dxdy);

    // dbgln("foo {}", dxdy);

    double x = p0.x();
    // if
    for (int y = p0.y(); y < (p1.y() - 1); y++) {
        int y_sub = y % 32;
        int y_bit = y / 32;
        int xi = (int)(x + offsets[y_sub]);
        u32 sample = 1 << y_sub;
        int idx = y_bit * m_size.width() + xi;

        m_data[idx] ^= sample;

        // if (idx < (int)m_data.size()) {
        // if (idx == 3569)
        //   dbgln("{:04} - {:08b} {:08b} {:08b}", idx, data, sample, m_data[idx]);
        // }
        x += dxdy;
    }
}

}
