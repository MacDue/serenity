/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/SVG/AttributeNames.h>
#include <LibWeb/SVG/SVGGradientElement.h>
#include <LibWeb/SVG/SVGGraphicsElement.h>

namespace Web::SVG {

SVGGradientElement::SVGGradientElement(DOM::Document& document, DOM::QualifiedName qualified_name)
    : SVGElement(document, move(qualified_name))
{
}

void SVGGradientElement::parse_attribute(DeprecatedFlyString const& name, DeprecatedString const& value)
{
    SVGElement::parse_attribute(name, value);
    if (name == AttributeNames::gradientUnits) {
        m_gradient_units = AttributeParser::parse_gradient_units(value);
    } else if (name == AttributeNames::gradientTransform) {
        if (auto transform_list = AttributeParser::parse_transform(value); transform_list.has_value()) {
            m_gradient_transform = transform_from_transform_list(*transform_list);
        } else {
            m_gradient_transform = {};
        }
    }
}

GradientUnits SVGGradientElement::gradient_units() const
{
    if (m_gradient_units.has_value())
        return *m_gradient_units;
    if (auto href = xlink_href())
        return href->gradient_units();
    return GradientUnits::ObjectBoundingBox;
}

Optional<Gfx::AffineTransform> SVGGradientElement::gradient_transform() const
{
    if (m_gradient_transform.has_value())
        return m_gradient_transform;
    if (auto href = xlink_href())
        return href->gradient_transform();
    return {};
}

// The gradient transform, appropriately scaled and combined with the paint transform.
Gfx::AffineTransform SVGGradientElement::gradient_paint_transform(SVGPaintContext const& paint_context) const
{
    auto transform = gradient_transform().value_or(Gfx::AffineTransform {});
    if (gradient_units() == GradientUnits::ObjectBoundingBox) {
        // Adjust transform to take place in the coordinate system defined by the bounding box:
        transform = Gfx::AffineTransform {}
                        .translate(paint_context.path_bounding_box.location())
                        .scale(paint_context.path_bounding_box.width(), paint_context.path_bounding_box.height())
                        .multiply(transform)
                        .scale(1 / paint_context.path_bounding_box.width(), 1 / paint_context.path_bounding_box.height())
                        .translate(-paint_context.path_bounding_box.location());
    }
    return Gfx::AffineTransform { paint_context.transform }.multiply(transform);
}

JS::GCPtr<SVGGradientElement const> SVGGradientElement::xlink_href() const
{
    // FIXME: This entire function is an ad-hoc hack!
    // It can only resolve #<ids> in the same document.
    if (auto href = get_attribute("href"); !href.is_empty()) {
        auto url = document().parse_url(href);
        auto id = url.fragment();
        if (id.is_empty())
            return {};
        auto element = document().get_element_by_id(id);
        if (!element)
            return {};
        if (!is<SVGGradientElement>(*element))
            return {};
        return &verify_cast<SVGGradientElement>(*element);
    }
    return {};
}

JS::ThrowCompletionOr<void> SVGGradientElement::initialize(JS::Realm& realm)
{
    MUST_OR_THROW_OOM(Base::initialize(realm));
    set_prototype(&Bindings::ensure_web_prototype<Bindings::SVGGradientElementPrototype>(realm, "SVGGradientElement"));
    return {};
}

}
