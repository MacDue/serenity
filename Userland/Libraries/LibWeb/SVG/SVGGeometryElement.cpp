/*
 * Copyright (c) 2020, Matthew Olsson <mattco@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/Layout/SVGGeometryBox.h>
#include <LibWeb/SVG/SVGGeometryElement.h>

namespace Web::SVG {

SVGGeometryElement::SVGGeometryElement(DOM::Document& document, DOM::QualifiedName qualified_name)
    : SVGGraphicsElement(document, move(qualified_name))
{
}

JS::ThrowCompletionOr<void> SVGGeometryElement::initialize(JS::Realm& realm)
{
    MUST_OR_THROW_OOM(Base::initialize(realm));
    set_prototype(&Bindings::ensure_web_prototype<Bindings::SVGGeometryElementPrototype>(realm, "SVGGeometryElement"));

    return {};
}

JS::GCPtr<Layout::Node> SVGGeometryElement::create_layout_node(NonnullRefPtr<CSS::StyleProperties> style)
{
    return heap().allocate_without_realm<Layout::SVGGeometryBox>(document(), *this, move(style));
}

Gfx::AffineTransform transform_from_transform_list(ReadonlySpan<Transform> tranform_list)
{
    Gfx::AffineTransform affine_transform;
    for (auto& tranform : tranform_list) {
        tranform.operation.visit(
            [&](Transform::Translate const& translate) {
                affine_transform.multiply(Gfx::AffineTransform {}.translate({ translate.x, translate.y }));
            },
            [&](Transform::Scale const& scale) {
                affine_transform.multiply(Gfx::AffineTransform {}.scale({ scale.x, scale.y }));
            },
            [&](Transform::Rotate const& rotate) {
                Gfx::AffineTransform translate_transform;
                affine_transform.multiply(
                    Gfx::AffineTransform {}
                        .translate({ -rotate.x, -rotate.y })
                        .rotate_radians(rotate.a * (AK::Pi<float> / 180.0f))
                        .translate({ rotate.x, rotate.y }));
            },
            [&](Transform::SkewX const& skew_x) {
                affine_transform.multiply(Gfx::AffineTransform {}.set_skew(skew_x.a, 0));
            },
            [&](Transform::SkewY const& skew_y) {
                affine_transform.multiply(Gfx::AffineTransform {}.set_skew(0, skew_y.a));
            },
            [&](Transform::Matrix const& matrix) {
                affine_transform.multiply(Gfx::AffineTransform {
                    matrix.a, matrix.b, matrix.c, matrix.d, matrix.e, matrix.f });
            });
    }
    return affine_transform;
}

void SVGGeometryElement::parse_attribute(DeprecatedFlyString const& name, DeprecatedString const& value)
{
    SVGGraphicsElement::parse_attribute(name, value);
    if (name == "transform"sv) {
        auto transform_list = AttributeParser::parse_transform(value);
        if (transform_list.has_value()) {
            dbgln("Parsed transform: {}", value);
            m_transform = transform_from_transform_list(*transform_list);
        }
    }
}

Gfx::AffineTransform SVGGeometryElement::get_transform() const
{
    return m_transform;
}

float SVGGeometryElement::get_total_length()
{
    return 0;
}

JS::NonnullGCPtr<Geometry::DOMPoint> SVGGeometryElement::get_point_at_length(float distance)
{
    (void)distance;
    return Geometry::DOMPoint::construct_impl(realm(), 0, 0, 0, 0).release_value_but_fixme_should_propagate_errors();
}

}
