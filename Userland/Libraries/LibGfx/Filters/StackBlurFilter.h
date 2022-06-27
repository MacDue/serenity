#pragma once

/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibGfx/Bitmap.h>

namespace Gfx {


class StackBlurFilter {
public:
    StackBlurFilter(Bitmap& bitmap)
        : m_bitmap(bitmap)
    {
    }

    void process_rgba(size_t radius, Color fill_color = Color::NamedColor::White);

private:
    Bitmap& m_bitmap;
};

}
