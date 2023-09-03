/*
 * Copyright (c) 2018-2022, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Layout/ImageBox.h>
#include <LibWeb/Painting/SVGGraphicsPaintable.h>
#include <LibWeb/SVG/SVGMaskElement.h>

namespace Web::Painting {

JS::NonnullGCPtr<SVGGraphicsPaintable> SVGGraphicsPaintable::create(Layout::SVGGraphicsBox const& layout_box)
{
    return layout_box.heap().allocate_without_realm<SVGGraphicsPaintable>(layout_box);
}

SVGGraphicsPaintable::SVGGraphicsPaintable(Layout::SVGGraphicsBox const& layout_box)
    : SVGPaintable(layout_box)
{
}

Layout::SVGGraphicsBox const& SVGGraphicsPaintable::layout_box() const
{
    return static_cast<Layout::SVGGraphicsBox const&>(layout_node());
}

bool SVGGraphicsPaintable::is_visible(Optional<PaintContext const&> context) const
{
    auto base_visibility = Base::is_visible(context);
    if (!base_visibility)
        return false;
    auto const* parent_mask_element = layout_box().dom_node().shadow_including_first_ancestor_of_type<SVG::SVGMaskElement>();
    if (parent_mask_element) {
        return context.has_value() && context->is_svg_mask_painting();
    }
    return true;
}

}
