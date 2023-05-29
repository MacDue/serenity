/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Array.h>
#include <AK/IntegralMath.h>
#include <AK/Types.h>
#include <LibGfx/EdgeFlagPathRasterizer.h>
#include <LibGfx/Painter.h>

#if defined(AK_COMPILER_GCC)
#    pragma GCC optimize("O3")
#endif

// This a very naive implementation of edge-flag scanline AA.
// The paper lists many possible optimizations, maybe implement one? (FIXME!)
// https://mlab.taik.fi/~kkallio/antialiasing/EdgeFlagAA.pdf

namespace Gfx {

template<unsigned SamplesPerPixel>
EdgeFlagPathRasterizer<SamplesPerPixel>::EdgeFlagPathRasterizer(Gfx::IntSize size)
    : m_size(size.width() + 1, size.height())
{
    m_data.resize(m_size.width() * m_size.height());
}

template<unsigned SamplesPerPixel>
RefPtr<Gfx::Bitmap> EdgeFlagPathRasterizer<SamplesPerPixel>::accumulate()
{
    auto result = MUST(Gfx::Bitmap::create(BitmapFormat::BGRA8888, m_size));
    constexpr auto alpha_shift = AK::log2(256 / SamplesPerPixel);
    for (int y = 0; y < m_size.height(); y += 1) {
        SampleType sample = 0;
        for (int x = 0; x < m_size.width(); x += 1) {
            sample ^= m_data[y * m_size.width() + x];
            auto coverage = SubpixelSample::compute_coverage(sample);
            if (coverage) {
                auto alpha = (coverage << alpha_shift) - 1;
                (void)alpha;
                result->set_pixel(x, y, Color(Color::Black).with_alpha(alpha));
            }
        }
    }
    return result;
}

template<unsigned SamplesPerPixel>
void EdgeFlagPathRasterizer<SamplesPerPixel>::draw_path(Gfx::Path& path)
{
    for (auto& line : path.split_lines())
        draw_line(line.from, line.to);
}

template<unsigned SamplesPerPixel>
void EdgeFlagPathRasterizer<SamplesPerPixel>::draw_line(Gfx::FloatPoint p0, Gfx::FloatPoint p1)
{
    p0.scale_by(1, SamplesPerPixel);
    p1.scale_by(1, SamplesPerPixel);

    if (p0.y() > p1.y())
        swap(p0, p1);

    if (p0.y() == p1.y())
        return;

    auto dx = p1.x() - p0.x();
    auto dy = p1.y() - p0.y();
    float dxdy = float(dx) / dy;
    float x = p0.x();

    for (int y = p0.y(); y < (p1.y() - 1); y++) {
        int y_sub = y % SamplesPerPixel;
        int y_bit = y / SamplesPerPixel;
        int xi = static_cast<int>(x + SubpixelSample::nrooks_subpixel_offsets[y_sub]);
        SampleType sample = 1 << y_sub;
        int idx = y_bit * m_size.width() + xi;
        m_data[idx] ^= sample;
        x += dxdy;
    }
}

template class EdgeFlagPathRasterizer<8>;
template class EdgeFlagPathRasterizer<16>;
template class EdgeFlagPathRasterizer<32>;

}
