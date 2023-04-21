/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/*
 * Copyright (c) 2020, Matthew Olsson <mattco@serenityos.org>
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

JS::GCPtr<SVGGradientElement const> SVGGradientElement::xlink_href() const
{
    if (auto href = get_attribute("href"); href.starts_with('#')) {
        auto element = document().get_element_by_id(href.substring_view(1));
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
