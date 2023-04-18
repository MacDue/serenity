/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/SVG/AttributeNames.h>
#include <LibWeb/SVG/AttributeParser.h>
#include <LibWeb/SVG/SVGLinearGradientElement.h>

namespace Web::SVG {

SVGLinearGradientElement::SVGLinearGradientElement(DOM::Document& document, DOM::QualifiedName qualified_name)
    : SVGGradientElement(document, qualified_name)
{
}

JS::ThrowCompletionOr<void> SVGLinearGradientElement::initialize(JS::Realm& realm)
{
    MUST_OR_THROW_OOM(Base::initialize(realm));
    set_prototype(&Bindings::ensure_web_prototype<Bindings::SVGLinearGradientElementPrototype>(realm, "SVGLinearGradientElement"));

    return {};
}

void SVGLinearGradientElement::parse_attribute(DeprecatedFlyString const& name, DeprecatedString const& value)
{
    SVGGradientElement::parse_attribute(name, value);

    if (name == SVG::AttributeNames::x1) {
        m_x1 = AttributeParser::parse_coordinate(value);
    } else if (name == SVG::AttributeNames::y1) {
        m_y1 = AttributeParser::parse_coordinate(value);
    } else if (name == SVG::AttributeNames::x2) {
        m_x2 = AttributeParser::parse_coordinate(value);
    } else if (name == SVG::AttributeNames::y2) {
        m_y2 = AttributeParser::parse_coordinate(value);
    }
}

float SVGLinearGradientElement::start_x() const
{
    // If the attribute is not specified, the effect is as if a value of '0%' were specified.
    return m_x1.value_or(0);
}

float SVGLinearGradientElement::start_y() const
{
    // If the attribute is not specified, the effect is as if a value of '0%' were specified.
    return m_y1.value_or(0);
}

float SVGLinearGradientElement::end_x() const
{
    // If the attribute is not specified, the effect is as if a value of '100%' were specified.
    return m_x2.value_or(1);
}

float SVGLinearGradientElement::end_y() const
{
    // If the attribute is not specified, the effect is as if a value of '0%' were specified.
    return m_y2.value_or(0);
}

Gfx::PaintStyle const& SVGLinearGradientElement::to_gfx_paint_style() const
{
    VERIFY_NOT_REACHED();
}

// https://www.w3.org/TR/SVG11/pservers.html#LinearGradientElementX1Attribute
JS::NonnullGCPtr<SVGAnimatedLength> SVGLinearGradientElement::x1() const
{
    // FIXME: Create a proper animated value when animations are supported.
    auto base_length = SVGLength::create(realm(), 0, start_x()).release_value_but_fixme_should_propagate_errors();
    auto anim_length = SVGLength::create(realm(), 0, start_x()).release_value_but_fixme_should_propagate_errors();
    return SVGAnimatedLength::create(realm(), move(base_length), move(anim_length)).release_value_but_fixme_should_propagate_errors();
}

// https://www.w3.org/TR/SVG11/pservers.html#LinearGradientElementY1Attribute
JS::NonnullGCPtr<SVGAnimatedLength> SVGLinearGradientElement::y1() const
{
    // FIXME: Create a proper animated value when animations are supported.
    auto base_length = SVGLength::create(realm(), 0, start_y()).release_value_but_fixme_should_propagate_errors();
    auto anim_length = SVGLength::create(realm(), 0, start_y()).release_value_but_fixme_should_propagate_errors();
    return SVGAnimatedLength::create(realm(), move(base_length), move(anim_length)).release_value_but_fixme_should_propagate_errors();
}

// https://www.w3.org/TR/SVG11/pservers.html#LinearGradientElementX2Attribute
JS::NonnullGCPtr<SVGAnimatedLength> SVGLinearGradientElement::x2() const
{
    // FIXME: Create a proper animated value when animations are supported.
    auto base_length = SVGLength::create(realm(), 0, start_x()).release_value_but_fixme_should_propagate_errors();
    auto anim_length = SVGLength::create(realm(), 0, start_x()).release_value_but_fixme_should_propagate_errors();
    return SVGAnimatedLength::create(realm(), move(base_length), move(anim_length)).release_value_but_fixme_should_propagate_errors();
}

// https://www.w3.org/TR/SVG11/pservers.html#LinearGradientElementY2Attribute
JS::NonnullGCPtr<SVGAnimatedLength> SVGLinearGradientElement::y2() const
{
    // FIXME: Create a proper animated value when animations are supported.
    auto base_length = SVGLength::create(realm(), 0, end_y()).release_value_but_fixme_should_propagate_errors();
    auto anim_length = SVGLength::create(realm(), 0, end_y()).release_value_but_fixme_should_propagate_errors();
    return SVGAnimatedLength::create(realm(), move(base_length), move(anim_length)).release_value_but_fixme_should_propagate_errors();
}

}
