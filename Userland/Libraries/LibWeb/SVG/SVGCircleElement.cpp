/*
 * Copyright (c) 2022, Sam Atkins <atkinssj@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/SVG/AttributeNames.h>
#include <LibWeb/SVG/AttributeParser.h>
#include <LibWeb/SVG/SVGCircleElement.h>

namespace Web::SVG {

JS_DEFINE_ALLOCATOR(SVGCircleElement);

SVGCircleElement::SVGCircleElement(DOM::Document& document, DOM::QualifiedName qualified_name)
    : SVGGeometryElement(document, qualified_name)
{
}

void SVGCircleElement::initialize(JS::Realm& realm)
{
    Base::initialize(realm);
    set_prototype(&Bindings::ensure_web_prototype<Bindings::SVGCircleElementPrototype>(realm, "SVGCircleElement"_fly_string));
}

void SVGSVGElement::apply_presentational_hints(CSS::StyleProperties& style) const
{
    Base::apply_presentational_hints(style);

    auto cx_attribute = attribute(SVG::AttributeNames::cx);
    if (auto cx_value = parse_css_value(parsing_context, cx_attribute.value_or(String {}), CSS::PropertyID::Cx))
        style.set_property(CSS::PropertyID::Cx, cx_value.release_nonnull());

    auto cy_attribute = attribute(SVG::AttributeNames::cy);
    if (auto cy_value = parse_css_value(parsing_context, cy_attribute.value_or(String {}), CSS::PropertyID::Cy))
        style.set_property(CSS::PropertyID::Cy, cy_value.release_nonnull());

    auto r_attribute = attribute(SVG::AttributeNames::x);
    if (auto r_value = parse_css_value(parsing_context, r_attribute.value_or(String {}), CSS::PropertyID::R))
        style.set_property(CSS::PropertyID::R, r_value.release_nonnull());
}

Gfx::Path SVGCircleElement::get_path(CSSPixelSize viewport_size)
{
    auto* node = layout_node();
    auto cx = float(node->computed_style().cx().to_px(node, viewport_size.width));
    auto cy = float(node->computed_style().cy().to_px(node, viewport_size.height));
    auto r = float(node->computed_style().r().to_px(node, viewport_size.width));

    Gfx::Path path;

    // A zero radius disables rendering.
    if (r == 0)
        return {};

    bool large_arc = false;
    bool sweep = true;

    // 1. A move-to command to the point cx+r,cy;
    path.move_to({ cx + r, cy });

    // 2. arc to cx,cy+r;
    path.arc_to({ cx, cy + r }, r, large_arc, sweep);

    // 3. arc to cx-r,cy;
    path.arc_to({ cx - r, cy }, r, large_arc, sweep);

    // 4. arc to cx,cy-r;
    path.arc_to({ cx, cy - r }, r, large_arc, sweep);

    // 5. arc with a segment-completing close path operation.
    path.arc_to({ cx + r, cy }, r, large_arc, sweep);

    return path;
}

// https://www.w3.org/TR/SVG11/shapes.html#CircleElementCXAttribute
JS::NonnullGCPtr<SVGAnimatedLength> SVGCircleElement::cx() const
{
    // FIXME: Create a proper animated value when animations are supported.
    TODO();
}

// https://www.w3.org/TR/SVG11/shapes.html#CircleElementCYAttribute
JS::NonnullGCPtr<SVGAnimatedLength> SVGCircleElement::cy() const
{
    // FIXME: Create a proper animated value when animations are supported.
    TODO();
}

// https://www.w3.org/TR/SVG11/shapes.html#CircleElementRAttribute
JS::NonnullGCPtr<SVGAnimatedLength> SVGCircleElement::r() const
{
    // FIXME: Create a proper animated value when animations are supported.
    TODO();
}

}
