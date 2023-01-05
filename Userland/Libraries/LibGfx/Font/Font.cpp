/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Font/Font.h>

namespace Gfx {

GlyphRasterPosition GlyphRasterPosition::get_nearest_fit_for(FloatPoint position)
{
    constexpr auto subpixel_divisions = GlyphSubpixelOffset::subpixel_divisions();
    auto compute_offset = [](float pos, int& blit_pos, u8& subpixel_offset) {
        blit_pos = floor(pos);
        subpixel_offset = AK::ceil((pos - blit_pos) / (1.0f / subpixel_divisions));
        if (subpixel_offset >= subpixel_divisions) {
            blit_pos += 1;
            subpixel_offset = 0;
        }
    };
    int blit_x, blit_y;
    u8 subpixel_x, subpixel_y;
    compute_offset(position.x(), blit_x, subpixel_x);
    compute_offset(position.y(), blit_y, subpixel_y);
    return GlyphRasterPosition { { blit_x, blit_y }, { subpixel_x, subpixel_y } };
}

}
