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

// https://svgwg.org/svg2-draft/pservers.html#RadialGradientElementFXAttribute
NumberPercentage SVGRadialGradientElement::start_circle_x() const
{
    if (m_fx.has_value())
        return *m_fx;
    // If the element references an element that specifies a value for 'fx', then the value of 'fx' is
    // inherited from the referenced element.
    if (auto href = radial_gradient_xlink_href())
        return href->start_circle_x();
    // If attribute ‘fx’ is not specified, ‘fx’ will coincide with the presentational value of ‘cx’ for
    // the element whether the value for 'cx' was inherited or not.
    return end_circle_x();
}

// https://svgwg.org/svg2-draft/pservers.html#RadialGradientElementFYAttribute
NumberPercentage SVGRadialGradientElement::start_circle_y() const
{
    if (m_fy.has_value())
        return *m_fy;
    // If the element references an element that specifies a value for 'fy', then the value of 'fy' is
    // inherited from the referenced element.
    if (auto href = radial_gradient_xlink_href())
        return href->start_circle_y();
    // If attribute ‘fy’ is not specified, ‘fy’ will coincide with the presentational value of ‘cy’ for
    // the element whether the value for 'cy' was inherited or not.
    return end_circle_y();
}

// https://svgwg.org/svg2-draft/pservers.html#RadialGradientElementFRAttribute
NumberPercentage SVGRadialGradientElement::start_circle_radius() const
{
    // Note: A negative value is an error.
    if (m_fr.has_value() && m_fr->value() >= 0)
        return *m_fr;
    // if the element references an element that specifies a value for 'fr', then the value of
    // 'fr' is inherited from the referenced element.
    if (auto href = radial_gradient_xlink_href())
        return href->start_circle_radius();
    // If the attribute is not specified, the effect is as if a value of '0%' were specified.
    return NumberPercentage::create_percentage(0);
}

// https://svgwg.org/svg2-draft/pservers.html#RadialGradientElementCXAttribute
NumberPercentage SVGRadialGradientElement::end_circle_x() const
{
    if (m_cx.has_value())
        return *m_cx;
    if (auto href = radial_gradient_xlink_href())
        return href->end_circle_x();
    return NumberPercentage::create_percentage(50);
}

// https://svgwg.org/svg2-draft/pservers.html#RadialGradientElementCYAttribute
NumberPercentage SVGRadialGradientElement::end_circle_y() const
{
    if (m_cy.has_value())
        return *m_cy;
    if (auto href = radial_gradient_xlink_href())
        return href->end_circle_y();
    return NumberPercentage::create_percentage(50);
}

// https://svgwg.org/svg2-draft/pservers.html#RadialGradientElementRAttribute
NumberPercentage SVGRadialGradientElement::end_circle_radius() const
{
    // Note: A negative value is an error.
    if (m_r.has_value() && m_r->value() >= 0)
        return *m_r;
    if (auto href = radial_gradient_xlink_href())
        return href->end_circle_radius();
    return NumberPercentage::create_percentage(50);
}

Optional<Gfx::PaintStyle const&> SVGRadialGradientElement::to_gfx_paint_style(SVGPaintContext const& paint_context) const
{
    auto units = gradient_units();
    // Gfx::FloatPoint start_center;
    // float start_radius = 0.0f;
    // Gfx::FloatPoint end_center;
    // float end_radius = 0.0f;

    if (units == GradientUnits::ObjectBoundingBox) {

    } else {
    }

    if (!m_paint_style) {

    } else {
    }

    m_paint_style->set_gradient_transform(gradient_paint_transform(paint_context));
    return *m_paint_style;
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
