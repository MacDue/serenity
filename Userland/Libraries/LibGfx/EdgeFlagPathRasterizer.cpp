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

// This a pretty naive implementation of edge-flag scanline AA.
// The paper lists many possible optimizations, maybe implement one? (FIXME!)
// https://mlab.taik.fi/~kkallio/antialiasing/EdgeFlagAA.pdf
// This currently implements:
//      - The scanline buffer optimization (only allocate one scanline)
// Possible other optimizations according to the paper:
//      - Using fixed point numbers
//      - Edge tracking
//      - Mask tracking
//      - Loop unrolling (compilers might handle this better now, the paper is form 2007)

namespace Gfx {

template<unsigned SamplesPerPixel>
EdgeFlagPathRasterizer<SamplesPerPixel>::EdgeFlagPathRasterizer(Gfx::IntSize size)
    : m_size(size.width() + 1, size.height() + 1)
{
    m_scanline.resize(m_size.width());
    m_edge_table.resize(m_size.height());
}

template<unsigned SamplesPerPixel>
RefPtr<Gfx::Bitmap> EdgeFlagPathRasterizer<SamplesPerPixel>::fill_even_odd(Gfx::Path& path)
{
    auto& lines = path.split_lines();
    if (lines.is_empty())
        return nullptr;

    // FIXME: split_lines() gives similar information, but the form it's in is not that useful (and is const anyway).
    Vector<Edge> edges;
    edges.ensure_capacity(lines.size());
    for (auto& line : lines) {
        auto p0 = line.from;
        auto p1 = line.to;

        p0.scale_by(1, SamplesPerPixel);
        p1.scale_by(1, SamplesPerPixel);

        if (p0.y() > p1.y())
            swap(p0, p1);

        if (p0.y() == p1.y())
            continue;

        auto dx = p1.x() - p0.x();
        auto dy = p1.y() - p0.y();
        float dxdy = float(dx) / dy;
        float x = p0.x();
        edges.unchecked_append(Edge {
            x,
            static_cast<int>(p0.y()),
            static_cast<int>(p1.y()),
            dxdy,
            nullptr });
    }

    m_min_y = m_size.height();
    m_max_y = 0;
    for (auto& edge : edges) {
        int min_scanline = edge.min_y / SamplesPerPixel;
        int max_scanline = edge.max_y / SamplesPerPixel;

        // Create a linked-list of edges starting on this scanline:
        edge.next_edge = m_edge_table[min_scanline];
        m_edge_table[min_scanline] = &edge;

        m_min_y = min(m_min_y, min_scanline);
        m_max_y = max(m_max_y, max_scanline);
    }

    Edge* active_edges = nullptr;
    auto plot_edge = [&](Edge& edge, int start_subpixel_y, int end_subpixel_y) {
        // auto slope =
        for (int y = start_subpixel_y; y < end_subpixel_y; y++) {
            int xi = static_cast<int>(edge.x + SubpixelSample::nrooks_subpixel_offsets[y]);
            SampleType sample = 1 << y;
            m_scanline[xi] ^= sample;
            edge.x += edge.dxdy;
        }
    };

    auto y_subpixel = [](int y) {
        return y & (SamplesPerPixel - 1);
    };

    auto result = MUST(Gfx::Bitmap::create(BitmapFormat::BGRA8888, m_size));

    for (int y = m_min_y; y <= m_max_y; y++) {
        auto* current_edge = active_edges;
        Edge* prev_edge = nullptr;

        // Previous edges:
        while (current_edge) {
            int end_scanline = current_edge->max_y / SamplesPerPixel;
            if (y == end_scanline) {
                // This edge ends this scanline.
                plot_edge(*current_edge, 0, y_subpixel(current_edge->max_y));
                // Remove this edge from the AET
                current_edge = current_edge->next_edge;
                if (prev_edge)
                    prev_edge->next_edge = current_edge;
                else
                    active_edges = current_edge;
            } else {
                // This egde sticks around for a few more scanlines.
                plot_edge(*current_edge, 0, SamplesPerPixel);
                prev_edge = current_edge;
                current_edge = current_edge->next_edge;
            }
        }

        // New edges starting this line
        current_edge = m_edge_table[y];
        while (current_edge) {
            int end_scanline = current_edge->max_y / SamplesPerPixel;
            if (y == end_scanline) {
                // This edge will end this scanlines (no need to add to AET).
                plot_edge(*current_edge, y_subpixel(current_edge->min_y), y_subpixel(current_edge->max_y));
            } else {
                // This edge will live on for a few more scanlines.
                plot_edge(*current_edge, y_subpixel(current_edge->min_y), SamplesPerPixel);
                // Add this edge to the AET
                if (prev_edge)
                    prev_edge->next_edge = current_edge;
                else
                    active_edges = current_edge;
                prev_edge = current_edge;
            }
            current_edge = current_edge->next_edge;
        }

        SampleType sample = 0;
        constexpr auto alpha_shift = AK::log2(256 / SamplesPerPixel);
        for (int x = 0; x < m_size.width(); x += 1) {
            sample ^= m_scanline[x];
            auto coverage = SubpixelSample::compute_coverage(sample);
            if (coverage) {
                auto alpha = (coverage << alpha_shift) - 1;
                (void)alpha;
                result->set_pixel(x, y, Color(Color::Black).with_alpha(alpha));
            }
            m_scanline[x] = 0;
        }
    }

    return result;
}

template class EdgeFlagPathRasterizer<8>;
template class EdgeFlagPathRasterizer<16>;
template class EdgeFlagPathRasterizer<32>;

}
