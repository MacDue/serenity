/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Variant.h>
#include <LibGfx/Color.h>
#include <LibWeb/CSS/StyleValue.h>

namespace Web::CSS {

class AccentColor {
public:
    struct Auto { };

    AccentColor()
        : m_color(Auto {})
    {
    }
    AccentColor(StyleValue const& color)
        : m_color(color)
    {
    }

    static inline AccentColor make_auto()
    {
        return AccentColor {};
    }

    bool is_auto() const { return m_color.has<Auto>(); }

    Gfx::Color to_color(Layout::NodeWithStyle const& node, Gfx::Color fallback = Color::Black) const;

private:
    StyleValue const& style_value() const
    {
        return m_color.get<NonnullRefPtr<StyleValue const>>();
    }

    Variant<Auto, NonnullRefPtr<StyleValue const>> m_color;
};

}
