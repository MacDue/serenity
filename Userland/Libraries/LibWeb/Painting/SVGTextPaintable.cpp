/*
 * Copyright (c) 2023, Aliaksandr Kalenik <kalenik.aliaksandr@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Painting/SVGTextPaintable.h>
#include <LibWeb/SVG/SVGSVGElement.h>

namespace Web::Painting {

JS::NonnullGCPtr<SVGTextPaintable> SVGTextPaintable::create(Layout::SVGTextBox const& layout_box)
{
    return layout_box.heap().allocate_without_realm<SVGTextPaintable>(layout_box);
}

SVGTextPaintable::SVGTextPaintable(Layout::SVGTextBox const& layout_box)
    : SVGGraphicsPaintable(layout_box)
{
}

void SVGTextPaintable::paint(PaintContext& context, PaintPhase phase) const
{
    if (!is_visible())
        return;

    if (!layout_node().computed_values().fill().has_value())
        return;

    if (layout_node().computed_values().fill()->is_url()) {
        dbgln("FIXME: Using url() as fill is not supported for svg text");
        return;
    }

    SVGGraphicsPaintable::paint(context, phase);

    if (phase != PaintPhase::Foreground)
        return;

    auto const& dom_node = layout_box().dom_node();
    auto paint_transform = computed_transforms().svg_to_device_pixels_transform(context);
    auto& font = layout_box().font();
    auto text_rect = absolute_rect().to_type<int>();
    auto text_contents = dom_node.text_contents();

    Gfx::Path text_path;
    text_path.move_to(text_rect.bottom_left().to_type<float>());
    text_path.text(Utf8View { text_contents }, font);

    context.painter().fill_path({
        .path = text_path.copy_transformed(paint_transform),
        .color = layout_node().computed_values().fill()->as_color(),
        .winding_rule = Gfx::Painter::WindingRule::Nonzero,
    });
}

}
