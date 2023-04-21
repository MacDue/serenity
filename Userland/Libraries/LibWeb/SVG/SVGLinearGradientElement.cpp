/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/SVG/AttributeNames.h>
#include <LibWeb/SVG/AttributeParser.h>
#include <LibWeb/SVG/SVGLinearGradientElement.h>
#include <LibWeb/SVG/SVGStopElement.h>

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
        m_paint_style = nullptr;
    } else if (name == SVG::AttributeNames::y1) {
        m_y1 = AttributeParser::parse_coordinate(value);
        m_paint_style = nullptr;
    } else if (name == SVG::AttributeNames::x2) {
        m_x2 = AttributeParser::parse_coordinate(value);
        m_paint_style = nullptr;
    } else if (name == SVG::AttributeNames::y2) {
        m_y2 = AttributeParser::parse_coordinate(value);
        m_paint_style = nullptr;
    }

    dbgln("<linearGradient>: Parsed coords: {} {} {} {}", m_x1, m_y1, m_x2, m_y2);
}

float SVGLinearGradientElement::start_x() const
{
    if (m_x1.has_value())
        return *m_x1;
    if (auto href = linear_gradient_xlink_href())
        return href->start_x();
    // If the attribute is not specified, the effect is as if a value of '0%' were specified.
    return 0;
}

float SVGLinearGradientElement::start_y() const
{
    if (m_y1.has_value())
        return *m_y1;
    if (auto href = linear_gradient_xlink_href())
        return href->start_x();
    // If the attribute is not specified, the effect is as if a value of '0%' were specified.
    return 0;
}

float SVGLinearGradientElement::end_x() const
{
    if (m_x2.has_value())
        return *m_x2;
    if (auto href = linear_gradient_xlink_href())
        return href->start_x();
    // If the attribute is not specified, the effect is as if a value of '100%' were specified.
    return 1;
}

float SVGLinearGradientElement::end_y() const
{
    if (m_y2.has_value())
        return *m_y2;
    if (auto href = linear_gradient_xlink_href())
        return href->start_x();
    // If the attribute is not specified, the effect is as if a value of '0%' were specified.
    return 0;
}

Optional<Gfx::PaintStyle const&> SVGLinearGradientElement::to_gfx_paint_style(Gfx::AffineTransform const& transform) const
{
    if (m_paint_style)
        return *m_paint_style;

    auto to_gfx_gradient_units = [](GradientUnits units) {
        switch (units) {
        case GradientUnits::ObjectBoundingBox:
            dbgln("Units: ObjectBoundingBox");
            return Gfx::SVGGradientUnits::ObjectBoundingBox;
        case GradientUnits::UserSpaceOnUse:
            dbgln("Units: UserSpaceOnUse");
            return Gfx::SVGGradientUnits::UserSpaceOnUse;
        default:
            VERIFY_NOT_REACHED();
        }
    };

    // FIXME: Resolve default lengths for UserSpaceOnUse
    auto gradient_style = Gfx::SVGLinearGradientPaintStyle::create({ start_x(),
                                                                       start_y() },
        { end_x(), end_y() })
                              .release_value_but_fixme_should_propagate_errors();

    for_each_color_stop([&](auto& stop) {
        auto stop_offset = stop.stop_offset().value_or(0);
        auto stop_color = stop.stop_color().value_or(Gfx::Color::Black);
        gradient_style->add_color_stop(stop_offset, stop_color).release_value_but_fixme_should_propagate_errors();
    });

    // FIXME: This breaks on resize...(the paintstyle needs invalidating on layout changes)
    // FIXME: Make changes for both gradient_units
    gradient_style->set_gradient_units(to_gfx_gradient_units(gradient_units()));
    gradient_style->set_gradient_transform(
        (
            Gfx::AffineTransform { transform }.multiply(gradient_transform().value_or(Gfx::AffineTransform {})))
            .inverse()
            .value_or(Gfx::AffineTransform {}));
    m_paint_style = gradient_style;
    return *m_paint_style;
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
