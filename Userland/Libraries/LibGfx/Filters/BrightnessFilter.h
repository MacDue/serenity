/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/StringView.h>
#include <LibGfx/Filters/ColorFilter.h>

namespace Gfx {

class BrightnessFilter : public ColorFilter {
public:
    using ColorFilter::ColorFilter;
    virtual ~BrightnessFilter() = default;

    virtual StringView class_name() const override { return "BrightnessFilter"sv; }

    virtual bool amount_handled_in_filter() const override
    {
        return true;
    }

protected:
    Color convert_color(Color original) override
    {
        auto channel_func = [&](u8 channel) {
            return static_cast<u8>(clamp(round_to<int>(channel * m_amount), 0, 255));
        };
        return Gfx::Color {
            channel_func(original.red()),
            channel_func(original.green()),
            channel_func(original.blue()),
            original.alpha()
        };
    };
};

}
