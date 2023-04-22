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

    // FIXME: Should allow for `<number-percentage> | <length>` for x1, x2, y1, y2
    if (name == SVG::AttributeNames::x1) {
        m_x1 = AttributeParser::parse_number_percentage(value);
        m_paint_style = nullptr;
    } else if (name == SVG::AttributeNames::y1) {
        m_y1 = AttributeParser::parse_number_percentage(value);
        m_paint_style = nullptr;
    } else if (name == SVG::AttributeNames::x2) {
        m_x2 = AttributeParser::parse_number_percentage(value);
        m_paint_style = nullptr;
    } else if (name == SVG::AttributeNames::y2) {
        m_y2 = AttributeParser::parse_number_percentage(value);
        m_paint_style = nullptr;
    }
}

NumberPercentage SVGLinearGradientElement::start_x() const
{
    if (m_x1.has_value())
        return *m_x1;
    if (auto href = linear_gradient_xlink_href())
        return href->start_x();
    // If the attribute is not specified, the effect is as if a value of '0%' were specified.
    return NumberPercentage::create_percentage(0);
}

NumberPercentage SVGLinearGradientElement::start_y() const
{
    if (m_y1.has_value())
        return *m_y1;
    if (auto href = linear_gradient_xlink_href())
        return href->start_x();
    // If the attribute is not specified, the effect is as if a value of '0%' were specified.
    return NumberPercentage::create_percentage(0);
}

NumberPercentage SVGLinearGradientElement::end_x() const
{
    if (m_x2.has_value())
        return *m_x2;
    if (auto href = linear_gradient_xlink_href())
        return href->start_x();
    // If the attribute is not specified, the effect is as if a value of '100%' were specified.
    return NumberPercentage::create_percentage(100);
}

NumberPercentage SVGLinearGradientElement::end_y() const
{
    if (m_y2.has_value())
        return *m_y2;
    if (auto href = linear_gradient_xlink_href())
        return href->start_x();
    // If the attribute is not specified, the effect is as if a value of '0%' were specified.
    return NumberPercentage::create_percentage(0);
}

Optional<Gfx::PaintStyle const&> SVGLinearGradientElement::to_gfx_paint_style(SVGPaintContext const& paint_context) const
{
    auto units = gradient_units();
    if (!m_paint_style) {
        // FIXME: Resolve percentages properly
        Gfx::FloatPoint start_point {};
        Gfx::FloatPoint end_point {};
        if (units == GradientUnits::ObjectBoundingBox) {
            // For gradientUnits="objectBoundingBox" both "100%" and "1" are treated the same.
            start_point = paint_context.path_bounding_box.location() + Gfx::FloatPoint { start_x().value() * paint_context.path_bounding_box.width(), start_y().value() * paint_context.path_bounding_box.height() };
            end_point = paint_context.path_bounding_box.location() + Gfx::FloatPoint { end_x().value() * paint_context.path_bounding_box.width(), end_y().value() * paint_context.path_bounding_box.height() };
        } else {
            // GradientUnits::UserSpaceOnUse
            start_point = Gfx::FloatPoint {
                start_x().resolve_relative_to(paint_context.viewport.width()),
                start_y().resolve_relative_to(paint_context.viewport.height()),
            };
            end_point = Gfx::FloatPoint {
                end_x().resolve_relative_to(paint_context.viewport.width()),
                end_y().resolve_relative_to(paint_context.viewport.height()),
            };
        }
        m_paint_style = Gfx::SVGLinearGradientPaintStyle::create(start_point, end_point)
                            .release_value_but_fixme_should_propagate_errors();

        for_each_color_stop([&](auto& stop) {
            dbgln("Stop value {}", stop.stop_offset().value());
            m_paint_style->add_color_stop(stop.stop_offset().value(), stop.stop_color()).release_value_but_fixme_should_propagate_errors();
        });
    }

    auto gradient_affine_transform = gradient_transform().value_or(Gfx::AffineTransform {});

    if (units == GradientUnits::ObjectBoundingBox) {
        // Adjust origin of gradient transform to top corner of bounding box:
        gradient_affine_transform = Gfx::AffineTransform {}
                                        .translate(paint_context.path_bounding_box.location())
                                        .multiply(gradient_affine_transform)
                                        .translate(-paint_context.path_bounding_box.location());
    }

    m_paint_style->set_gradient_transform(Gfx::AffineTransform { paint_context.transform }.multiply(gradient_affine_transform));
    return *m_paint_style;
}

// https://www.w3.org/TR/SVG11/pservers.html#LinearGradientElementX1Attribute
JS::NonnullGCPtr<SVGAnimatedLength> SVGLinearGradientElement::x1() const
{
    TODO();
}

// https://www.w3.org/TR/SVG11/pservers.html#LinearGradientElementY1Attribute
JS::NonnullGCPtr<SVGAnimatedLength> SVGLinearGradientElement::y1() const
{
    TODO();
}

// https://www.w3.org/TR/SVG11/pservers.html#LinearGradientElementX2Attribute
JS::NonnullGCPtr<SVGAnimatedLength> SVGLinearGradientElement::x2() const
{
    TODO();
}

// https://www.w3.org/TR/SVG11/pservers.html#LinearGradientElementY2Attribute
JS::NonnullGCPtr<SVGAnimatedLength> SVGLinearGradientElement::y2() const
{
    TODO();
}

}
