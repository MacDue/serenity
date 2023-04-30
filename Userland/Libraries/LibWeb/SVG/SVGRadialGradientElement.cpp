/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/SVG/AttributeNames.h>
#include <LibWeb/SVG/SVGRadialGradientElement.h>

namespace Web::SVG {

SVGRadialGradientElement::SVGRadialGradientElement(DOM::Document& document, DOM::QualifiedName qualified_name)
    : SVGGradientElement(document, qualified_name)
{
}

JS::ThrowCompletionOr<void> SVGRadialGradientElement::initialize(JS::Realm& realm)
{
    MUST_OR_THROW_OOM(Base::initialize(realm));
    set_prototype(&Bindings::ensure_web_prototype<Bindings::SVGRadialGradientElementPrototype>(realm, "SVGRadialGradientElement"));

    return {};
}

void SVGRadialGradientElement::parse_attribute(DeprecatedFlyString const& name, DeprecatedString const& value)
{
    SVGGradientElement::parse_attribute(name, value);

    // FIXME: These are <length> or <coordinate> in the spec, but allow examples seem to allow percentages
    // and unitless values.
    if (name == SVG::AttributeNames::cx) {
        m_cx = AttributeParser::parse_number_percentage(value);
        m_paint_style = nullptr;
    } else if (name == SVG::AttributeNames::cy) {
        m_cy = AttributeParser::parse_number_percentage(value);
        m_paint_style = nullptr;
    } else if (name == SVG::AttributeNames::fx) {
        m_fx = AttributeParser::parse_number_percentage(value);
        m_paint_style = nullptr;
    } else if (name == SVG::AttributeNames::fy) {
        m_fy = AttributeParser::parse_number_percentage(value);
        m_paint_style = nullptr;
    } else if (name == SVG::AttributeNames::fr) {
        m_fr = AttributeParser::parse_number_percentage(value);
        m_paint_style = nullptr;
    } else if (name == SVG::AttributeNames::r) {
        m_r = AttributeParser::parse_number_percentage(value);
        m_paint_style = nullptr;
    }
}

JS::NonnullGCPtr<SVGAnimatedLength> SVGRadialGradientElement::cx() const
{
    TODO();
}

JS::NonnullGCPtr<SVGAnimatedLength> SVGRadialGradientElement::cy() const
{
    TODO();
}

JS::NonnullGCPtr<SVGAnimatedLength> SVGRadialGradientElement::fx() const
{
    TODO();
}

JS::NonnullGCPtr<SVGAnimatedLength> SVGRadialGradientElement::fy() const
{
    TODO();
}

JS::NonnullGCPtr<SVGAnimatedLength> SVGRadialGradientElement::fr() const
{
    TODO();
}

JS::NonnullGCPtr<SVGAnimatedLength> SVGRadialGradientElement::r() const
{
    TODO();
}

}
