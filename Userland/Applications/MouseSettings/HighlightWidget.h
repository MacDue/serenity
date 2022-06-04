/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibGUI/SettingsWindow.h>
#include <LibGUI/Window.h>
#include <LibGUI/ColorInput.h>
#include <LibGUI/Slider.h>

class HighlightWidget final : public GUI::SettingsWindow::Tab {
    C_OBJECT(HighlightWidget)
public:
    virtual ~HighlightWidget() override = default;

    virtual void apply_settings() override;
    virtual void reset_default_values() override;

private:
    HighlightWidget();
    RefPtr<GUI::ColorInput> m_highlight_color_input;
    RefPtr<GUI::Slider> m_highlight_opacity_slider;
    RefPtr<GUI::Slider> m_highlight_radius_slider;
};
