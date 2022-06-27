/*
 * Copyright (c) 2010, Mario Klingemann <mario@quasimondo.com>
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC optimize("O3")
#endif

#include <AK/Array.h>
#include <AK/CircularQueue.h>
#include <AK/Vector.h>
#include <LibGfx/Filters/StackBlurFilter.h>

namespace Gfx {

constexpr Array<u16, 255> mul_table {
    512, 512, 456, 512, 328, 456, 335, 512, 405, 328, 271, 456, 388, 335, 292,
    512, 454, 405, 364, 328, 298, 271, 496, 456, 420, 388, 360, 335, 312, 292,
    273, 512, 482, 454, 428, 405, 383, 364, 345, 328, 312, 298, 284, 271, 259,
    496, 475, 456, 437, 420, 404, 388, 374, 360, 347, 335, 323, 312, 302, 292,
    282, 273, 265, 512, 497, 482, 468, 454, 441, 428, 417, 405, 394, 383, 373,
    364, 354, 345, 337, 328, 320, 312, 305, 298, 291, 284, 278, 271, 265, 259,
    507, 496, 485, 475, 465, 456, 446, 437, 428, 420, 412, 404, 396, 388, 381,
    374, 367, 360, 354, 347, 341, 335, 329, 323, 318, 312, 307, 302, 297, 292,
    287, 282, 278, 273, 269, 265, 261, 512, 505, 497, 489, 482, 475, 468, 461,
    454, 447, 441, 435, 428, 422, 417, 411, 405, 399, 394, 389, 383, 378, 373,
    368, 364, 359, 354, 350, 345, 341, 337, 332, 328, 324, 320, 316, 312, 309,
    305, 301, 298, 294, 291, 287, 284, 281, 278, 274, 271, 268, 265, 262, 259,
    257, 507, 501, 496, 491, 485, 480, 475, 470, 465, 460, 456, 451, 446, 442,
    437, 433, 428, 424, 420, 416, 412, 408, 404, 400, 396, 392, 388, 385, 381,
    377, 374, 370, 367, 363, 360, 357, 354, 350, 347, 344, 341, 338, 335, 332,
    329, 326, 323, 320, 318, 315, 312, 310, 307, 304, 302, 299, 297, 294, 292,
    289, 287, 285, 282, 280, 278, 275, 273, 271, 269, 267, 265, 263, 261, 259
};

constexpr Array<u8, 255> shg_table {
    9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
    17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};

// Note: This is named to be consistent with the algorithm, but it's actually a simple circular buffer.
struct BlurStack {
    BlurStack(size_t size)
    {
        m_data.resize(size);
    }

    struct Iterator {
        friend BlurStack;

        ALWAYS_INLINE Color& operator*()
        {
            return m_data.at(m_idx);
        }

        ALWAYS_INLINE Color* operator->()
        {
            return &m_data.at(m_idx);
        }

        ALWAYS_INLINE Iterator operator++()
        {
            m_idx = (m_idx + 1) % m_data.size();
            return *this;
        }

        ALWAYS_INLINE Iterator operator++(int)
        {
            auto prev_it = *this;
            ++*(this);
            return prev_it;
        }

    private:
        Iterator(size_t idx, Span<Color> data)
            : m_idx(idx)
            , m_data(data)
        {
        }

        size_t m_idx;
        Span<Color> m_data;
    };

    Iterator iterator_from_position(size_t position)
    {
        VERIFY(position < m_data.size());
        return Iterator(position, m_data);
    }

private:
    Vector<Color, 512> m_data;
};

void StackBlurFilter::process_rgba(u8 radius, Color fill_color)
{
    // TODO: Implement a plain RGB version of this (if required)
    if (radius == 0)
        return;

    fill_color = fill_color.with_alpha(0);

    size_t width = m_bitmap.width();
    size_t height = m_bitmap.height();
    unsigned div = 2 * radius + 1;
    unsigned radius_plus_1 = radius + 1;
    unsigned sum_factor = radius_plus_1 * (radius_plus_1 + 1) / 2;

    auto get_pixel = [&](int x, int y) {
        auto color = m_bitmap.get_pixel<StorageFormat::BGRA8888>(x, y);
        if (color.alpha() == 0)
            return fill_color;
        return color;
    };

    auto set_pixel = [&](int x, int y, Color color) {
        return m_bitmap.set_pixel<StorageFormat::BGRA8888>(x, y, color);
    };

    BlurStack blur_stack { div };
    auto const stack_start = blur_stack.iterator_from_position(0);
    auto const stack_end = blur_stack.iterator_from_position(radius_plus_1);
    auto stack_iterator = stack_start;

    auto mul_sum = mul_table[radius];
    auto shg_sum = shg_table[radius];

    for (size_t y = 0; y < height; y++) {
        stack_iterator = stack_start;

        auto color = get_pixel(0, y);

        for (size_t i = 0; i < radius_plus_1; i++) {
            *stack_iterator = color;
            ++stack_iterator;
        }

        u32 red_in_sum = 0;
        u32 green_in_sum = 0;
        u32 blue_in_sum = 0;
        u32 alpha_in_sum = 0;
        u32 red_out_sum = radius_plus_1 * color.red();
        u32 green_out_sum = radius_plus_1 * color.green();
        u32 blue_out_sum = radius_plus_1 * color.blue();
        u32 alpha_out_sum = radius_plus_1 * color.alpha();
        u32 red_sum = sum_factor * color.red();
        u32 green_sum = sum_factor * color.green();
        u32 blue_sum = sum_factor * color.blue();
        u32 alpha_sum = sum_factor * color.alpha();

        for (u32 i = 1; i <= radius; i++) {
            auto color = get_pixel(min(i, width - 1), y);

            auto rbs = radius_plus_1 - i;
            *stack_iterator = color;
            red_sum += color.red() * rbs;
            green_sum += color.green() * rbs;
            blue_sum += color.blue() * rbs;
            alpha_sum += color.alpha() * rbs;

            red_in_sum += color.red();
            green_in_sum += color.green();
            blue_in_sum += color.blue();
            alpha_in_sum += color.alpha();

            ++stack_iterator;
        }

        auto stack_in_iterator = stack_start;
        auto stack_out_iterator = stack_end;

        for (size_t x = 0; x < width; x++) {
            auto alpha = (alpha_sum * mul_sum) >> shg_sum;
            if (alpha != 0)
                set_pixel(x, y, Color((red_sum * mul_sum) >> shg_sum, (green_sum * mul_sum) >> shg_sum, (blue_sum * mul_sum) >> shg_sum, alpha));
            else
                set_pixel(x, y, fill_color);

            red_sum -= red_out_sum;
            green_sum -= green_out_sum;
            blue_sum -= blue_out_sum;
            alpha_sum -= alpha_out_sum;

            red_out_sum -= stack_in_iterator->red();
            green_out_sum -= stack_in_iterator->green();
            blue_out_sum -= stack_in_iterator->blue();
            alpha_out_sum -= stack_in_iterator->alpha();

            auto color = get_pixel(min(x + radius_plus_1, width - 1), y);
            *stack_in_iterator = color;
            red_in_sum += color.red();
            green_in_sum += color.green();
            blue_in_sum += color.blue();
            alpha_in_sum += color.alpha();

            red_sum += red_in_sum;
            green_sum += green_in_sum;
            blue_sum += blue_in_sum;
            alpha_sum += alpha_in_sum;

            ++stack_in_iterator;

            color = *stack_out_iterator;
            red_out_sum += color.red();
            green_out_sum += color.green();
            blue_out_sum += color.blue();
            alpha_out_sum += color.alpha();

            red_in_sum -= color.red();
            green_in_sum -= color.green();
            blue_in_sum -= color.blue();
            alpha_in_sum -= color.alpha();

            ++stack_out_iterator;
        }
    }

    for (size_t x = 0; x < width; x++) {
        auto color = get_pixel(x, 0);

        for (size_t i = 0; i < radius_plus_1; i++) {
            *stack_iterator = color;
            ++stack_iterator;
        }

        u32 red_in_sum = 0;
        u32 green_in_sum = 0;
        u32 blue_in_sum = 0;
        u32 alpha_in_sum = 0;
        u32 red_out_sum = radius_plus_1 * color.red();
        u32 green_out_sum = radius_plus_1 * color.green();
        u32 blue_out_sum = radius_plus_1 * color.blue();
        u32 alpha_out_sum = radius_plus_1 * color.alpha();
        u32 red_sum = sum_factor * color.red();
        u32 green_sum = sum_factor * color.green();
        u32 blue_sum = sum_factor * color.blue();
        u32 alpha_sum = sum_factor * color.alpha();

        stack_iterator = stack_start;

        for (size_t i = 0; i < radius_plus_1; i++) {
            *stack_iterator = color;
            ++stack_iterator;
        }

        for (size_t i = 1; i <= radius; i++) {
            auto color = get_pixel(x, min(i, height - 1));

            auto rbs = radius_plus_1 - i;
            *stack_iterator = color;
            red_sum += color.red() * rbs;
            green_sum += color.green() * rbs;
            blue_sum += color.blue() * rbs;
            alpha_sum += color.alpha() * rbs;

            red_in_sum += color.red();
            green_in_sum += color.green();
            blue_in_sum += color.blue();
            alpha_in_sum += color.alpha();

            ++stack_iterator;
        }

        auto stack_in_iterator = stack_start;
        auto stack_out_iterator = stack_end;

        for (size_t y = 0; y < height; y++) {
            auto alpha = (alpha_sum * mul_sum) >> shg_sum;
            if (alpha != 0)
                set_pixel(x, y, Color((red_sum * mul_sum) >> shg_sum, (green_sum * mul_sum) >> shg_sum, (blue_sum * mul_sum) >> shg_sum, alpha));
            else
                set_pixel(x, y, fill_color);

            red_sum -= red_out_sum;
            green_sum -= green_out_sum;
            blue_sum -= blue_out_sum;
            alpha_sum -= alpha_out_sum;

            red_out_sum -= stack_in_iterator->red();
            green_out_sum -= stack_in_iterator->green();
            blue_out_sum -= stack_in_iterator->blue();
            alpha_out_sum -= stack_in_iterator->alpha();

            auto color = get_pixel(x, min(y + radius_plus_1, height - 1));
            *stack_in_iterator = color;
            red_in_sum += color.red();
            green_in_sum += color.green();
            blue_in_sum += color.blue();
            alpha_in_sum += color.alpha();

            red_sum += red_in_sum;
            green_sum += green_in_sum;
            blue_sum += blue_in_sum;
            alpha_sum += alpha_in_sum;

            ++stack_in_iterator;

            color = *stack_out_iterator;
            red_out_sum += color.red();
            green_out_sum += color.green();
            blue_out_sum += color.blue();
            alpha_out_sum += color.alpha();

            red_in_sum -= color.red();
            green_in_sum -= color.green();
            blue_in_sum -= color.blue();
            alpha_in_sum -= color.alpha();

            ++stack_out_iterator;
        }
    }
}

}
