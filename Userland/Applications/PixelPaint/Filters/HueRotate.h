/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "Filter.h"

namespace PixelPaint::Filters {

class HueRotate final : public Filter {
public:
    virtual void apply(Gfx::Bitmap& target_bitmap, Gfx::Bitmap const& source_bitmap) const override;
    virtual RefPtr<GUI::Widget> get_settings_widget() override;

    virtual StringView filter_name() const override { return "Hue Rotate"sv; }

    HueRotate(ImageEditor* editor)
        : Filter(editor) {};

private:
    float m_angle { 0 };
};

}
