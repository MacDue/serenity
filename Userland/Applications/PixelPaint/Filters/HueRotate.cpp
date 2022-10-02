/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "HueRotate.h"
#include "../FilterParams.h"
#include <LibGUI/Label.h>
#include <LibGUI/ValueSlider.h>
#include <LibGfx/Filters/HueRotateFilter.h>

namespace PixelPaint::Filters {

void HueRotate::apply(Gfx::Bitmap& target_bitmap, Gfx::Bitmap const& source_bitmap) const
{
    Gfx::HueRotateFilter filter { m_angle + 360 };
    filter.apply(target_bitmap, target_bitmap.rect(), source_bitmap, source_bitmap.rect());
}

RefPtr<GUI::Widget> HueRotate::get_settings_widget()
{
    if (!m_settings_widget) {
        m_settings_widget = GUI::Widget::construct();
        m_settings_widget->set_layout<GUI::VerticalBoxLayout>();

        auto& name_label = m_settings_widget->add<GUI::Label>("Angle");
        name_label.set_font_weight(Gfx::FontWeight::Bold);
        name_label.set_text_alignment(Gfx::TextAlignment::CenterLeft);
        name_label.set_fixed_height(20);

        auto& hue_angle_slider = m_settings_widget->add<GUI::ValueSlider>(Orientation::Horizontal);
        hue_angle_slider.set_range(-180, 180);
        hue_angle_slider.set_value(m_angle);
        hue_angle_slider.on_change = [&](int value) {
            m_angle = value;
            update_preview();
        };
    }

    return m_settings_widget;
}

}
