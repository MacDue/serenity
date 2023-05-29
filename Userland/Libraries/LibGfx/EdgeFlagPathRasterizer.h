/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Array.h>
#include <AK/Vector.h>
#include <LibGfx/Bitmap.h>
#include <LibGfx/Path.h>

namespace Gfx {

namespace Detail {

static auto constexpr coverage_lut = [] {
    Array<u8, 256> coverage_lut {};
    for (u32 sample = 0; sample <= 255; sample++)
        coverage_lut[sample] = popcount(sample);
    return coverage_lut;
}();

template<unsigned>
struct Sample {
};

template<>
struct Sample<32> {
    using Type = u32;
    static constexpr Array nrooks_subpixel_offsets {
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

    static u8 compute_coverage(Type sample)
    {
        return (
            coverage_lut[(sample >> 0) & 0xff]
            + coverage_lut[(sample >> 8) & 0xff]
            + coverage_lut[(sample >> 16) & 0xff]
            + coverage_lut[(sample >> 24) & 0xff]);
    }
};

template<>
struct Sample<16> {
    using Type = u16;
    static constexpr Array nrooks_subpixel_offsets {
        (1.0f / 16.0f),
        (8.0f / 16.0f),
        (4.0f / 16.0f),
        (15.0f / 16.0f),
        (11.0f / 16.0f),
        (2.0f / 16.0f),
        (6.0f / 16.0f),
        (14.0f / 16.0f),
        (10.0f / 16.0f),
        (3.0f / 16.0f),
        (7.0f / 16.0f),
        (12.0f / 16.0f),
        (0.0f / 16.0f),
        (9.0f / 16.0f),
        (5.0f / 16.0f),
        (13.0f / 16.0f),
    };

    static u8 compute_coverage(Type sample)
    {
        return (
            coverage_lut[(sample >> 0) & 0xff]
            + coverage_lut[(sample >> 8) & 0xff]);
    }
};

template<>
struct Sample<8> {
    using Type = u8;
    static constexpr Array nrooks_subpixel_offsets {
        (5.0f / 8.0f),
        (0.0f / 8.0f),
        (3.0f / 8.0f),
        (6.0f / 8.0f),
        (1.0f / 8.0f),
        (4.0f / 8.0f),
        (7.0f / 8.0f),
        (2.0f / 8.0f),
    };

    static u8 compute_coverage(Type sample)
    {
        return coverage_lut[sample];
    }
};

}

template<unsigned SamplesPerPixel = 32>
class EdgeFlagPathRasterizer {
public:
    EdgeFlagPathRasterizer(Gfx::IntSize);
    void draw_path(Gfx::Path&);
    RefPtr<Gfx::Bitmap> accumulate();

private:
    using SubpixelSample = Detail::Sample<SamplesPerPixel>;
    using SampleType = typename SubpixelSample::Type;

    void draw_line(Gfx::FloatPoint, Gfx::FloatPoint);

    Gfx::IntSize m_size;
    Vector<SampleType> m_data;
};

extern template class EdgeFlagPathRasterizer<8>;
extern template class EdgeFlagPathRasterizer<16>;
extern template class EdgeFlagPathRasterizer<32>;

}
