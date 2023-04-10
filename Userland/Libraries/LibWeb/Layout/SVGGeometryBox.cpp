/*
 * Copyright (c) 2020, Matthew Olsson <matthewcolsson@gmail.com>
 * Copyright (c) 2022, Tobias Christiansen <tobyase@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Layout/SVGGeometryBox.h>
#include <LibWeb/Painting/SVGGeometryPaintable.h>
#include <LibWeb/SVG/SVGPathElement.h>
#include <LibWeb/SVG/SVGSVGElement.h>

namespace Web::Layout {

SVGGeometryBox::SVGGeometryBox(DOM::Document& document, SVG::SVGGeometryElement& element, NonnullRefPtr<CSS::StyleProperties> properties)
    : SVGGraphicsBox(document, element, properties)
{
}

float SVGGeometryBox::viewbox_scaling() const
{
    auto* svg_box = dom_node().first_ancestor_of_type<SVG::SVGSVGElement>();

    if (!svg_box || !svg_box->view_box().has_value())
        return 1;

    auto view_box = svg_box->view_box().value();
    bool has_specified_width = svg_box->has_attribute(HTML::AttributeNames::width);
    bool has_specified_height = svg_box->has_attribute(HTML::AttributeNames::height);
    auto scale_width = has_specified_width ? svg_box->paint_box()->content_width().value() / view_box.width : 1;
    auto scale_height = has_specified_height ? svg_box->paint_box()->content_height().value() / view_box.height : 1;
    return min(scale_width, scale_height);
}

CSSPixelPoint SVGGeometryBox::viewbox_origin() const
{
    auto* svg_box = dom_node().first_ancestor_of_type<SVG::SVGSVGElement>();
    if (!svg_box || !svg_box->view_box().has_value())
        return { 0, 0 };
    return { svg_box->view_box().value().min_x, svg_box->view_box().value().min_y };
}

Gfx::AffineTransform SVGGeometryBox::paint_transform() const
{
    auto& geometry_element = dom_node();
    auto transform = geometry_element.get_transform();
    auto scaling = viewbox_scaling();
    auto origin = viewbox_origin();
    return Gfx::AffineTransform {}.scale(scaling, scaling).translate(-origin.to_type<float>()).multiply(transform);
}

JS::GCPtr<Painting::Paintable> SVGGeometryBox::create_paintable() const
{
    return Painting::SVGGeometryPaintable::create(*this);
}

}
