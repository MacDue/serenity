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
//      - Loop unrolling (compilers might handle this better now, the paper is from 2007)

namespace Gfx {

template<unsigned SamplesPerPixel>
EdgeFlagPathRasterizer<SamplesPerPixel>::EdgeFlagPathRasterizer(Gfx::IntSize size)
    : m_size(size.width() + 1, size.height() + 1)
{
    m_scanline.resize(m_size.width());
    m_edge_table.resize(m_size.height());
}

static Vector<Detail::Edge> prepare_edges(ReadonlySpan<Path::SplitLineSegment> lines, unsigned samples_per_pixel)
{
    // FIXME: split_lines() gives similar information, but the form it's in is not that useful (and is const anyway).
    Vector<Detail::Edge> edges;
    edges.ensure_capacity(lines.size());
    for (auto& line : lines) {
        auto p0 = line.from;
        auto p1 = line.to;

        p0.scale_by(1, samples_per_pixel);
        p1.scale_by(1, samples_per_pixel);

        if (p0.y() > p1.y())
            swap(p0, p1);

        if (p0.y() == p1.y())
            continue;

        auto dx = p1.x() - p0.x();
        auto dy = p1.y() - p0.y();
        float dxdy = float(dx) / dy;
        float x = p0.x();
        edges.unchecked_append(Detail::Edge {
            x,
            static_cast<int>(p0.y()),
            static_cast<int>(p1.y()),
            dxdy,
            nullptr });
    }
    return edges;
}

template<unsigned SamplesPerPixel>
Detail::Edge* EdgeFlagPathRasterizer<SamplesPerPixel>::plot_edges_for_scanline(int scanline, Detail::Edge* active_edges)
{
    auto plot_edge = [&](Detail::Edge& edge, int start_subpixel_y, int end_subpixel_y) {
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

    auto* current_edge = active_edges;
    Detail::Edge* prev_edge = nullptr;

    // First iterate over the edge in the active edge table, these are edges added on earlier scanlines,
    // that have not yet reached their end scanline.
    while (current_edge) {
        int end_scanline = current_edge->max_y / SamplesPerPixel;
        if (scanline == end_scanline) {
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

    // Next, iterate over new edges for this line. If active_edges was null this also becomes the new
    // AET. Edges new will be appended here.
    current_edge = m_edge_table[scanline];
    while (current_edge) {
        int end_scanline = current_edge->max_y / SamplesPerPixel;
        if (scanline == end_scanline) {
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

    return active_edges;
}

template<unsigned SamplesPerPixel>
void EdgeFlagPathRasterizer<SamplesPerPixel>::accumulate_scanline(Gfx::Bitmap& result, int scanline)
{
    SampleType sample = 0;
    constexpr auto alpha_shift = AK::log2(256 / SamplesPerPixel);
    for (int x = 0; x < m_size.width(); x += 1) {
        sample ^= m_scanline[x];
        auto coverage = SubpixelSample::compute_coverage(sample);
        if (coverage) {
            auto alpha = (coverage << alpha_shift) - 1;
            result.set_pixel(x, scanline, Color(Color::Black).with_alpha(alpha));
        }
        m_scanline[x] = 0;
    }
}

template<unsigned SamplesPerPixel>
RefPtr<Gfx::Bitmap> EdgeFlagPathRasterizer<SamplesPerPixel>::fill_even_odd(Gfx::Path& path)
{
    auto& lines = path.split_lines();
    if (lines.is_empty())
        return nullptr;

    auto edges = prepare_edges(lines, SamplesPerPixel);

    int min_scanline = m_size.height();
    int max_scanline = 0;
    for (auto& edge : edges) {
        int start_scanline = edge.min_y / SamplesPerPixel;
        int end_scanline = edge.max_y / SamplesPerPixel;

        // Create a linked-list of edges starting on this scanline:
        edge.next_edge = m_edge_table[start_scanline];
        m_edge_table[start_scanline] = &edge;

        min_scanline = min(min_scanline, start_scanline);
        max_scanline = max(max_scanline, end_scanline);
    }

    auto result = MUST(Gfx::Bitmap::create(BitmapFormat::BGRA8888, m_size));

    Detail::Edge* active_edges = nullptr;
    for (int scanline = min_scanline; scanline <= max_scanline; scanline++) {
        active_edges = plot_edges_for_scanline(scanline, active_edges);
        accumulate_scanline(*result, scanline);
    }

    return result;
}

template class EdgeFlagPathRasterizer<8>;
template class EdgeFlagPathRasterizer<16>;
template class EdgeFlagPathRasterizer<32>;

}
