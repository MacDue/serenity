/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/CSS/AccentColor.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/Layout/Node.h>

namespace Web::CSS {

Gfx::Color AccentColor::to_color(Layout::NodeWithStyle const& node, Gfx::Color fallback) const
{
    if (is_auto() || !style_value().has_color()) {
        auto& document = node.document();
        if (!document.page())
            return fallback;
        return document.page()->palette().color(ColorRole::Accent);
    }
    return style_value().to_color(node);
}

}
