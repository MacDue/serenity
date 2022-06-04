/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "HighlightWidget.h"

#include <Applications/MouseSettings/HighlightWidgetGML.h>
#include <LibGUI/ConnectionToWindowServer.h>

HighlightWidget::HighlightWidget()
{
    load_from_gml(highlight_widget_gml);
    m_highlight_color_input = *find_descendant_of_type_named<GUI::ColorInput>("highlight_color_input");
    m_highlight_color_input->set_color(Gfx::Color::NamedColor::Yellow);
    m_highlight_color_input->on_change = [&]() {
      set_modified(true);
    };

    m_highlight_opacity_slider = *find_descendant_of_type_named<GUI::Slider>("highlight_opacity_slider");
    m_highlight_opacity_slider->on_change = [&](int) {
      set_modified(true);
    };

    m_highlight_radius_slider = *find_descendant_of_type_named<GUI::Slider>("highlight_radius_slider");
    m_highlight_radius_slider->on_change = [&](int) {
      set_modified(true);
    };
}

void HighlightWidget::apply_settings()
{
  auto color = m_highlight_color_input->color();
  auto color_alpha = (m_highlight_opacity_slider->value()/100.) * 255;
  color.set_alpha(color_alpha);
  GUI::ConnectionToWindowServer::the().async_set_cursor_highlight(m_highlight_radius_slider->value(), color);
}

void HighlightWidget::reset_default_values()
{
}
