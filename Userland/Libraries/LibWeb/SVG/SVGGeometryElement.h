/*
 * Copyright (c) 2020, Matthew Olsson <mattco@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibGfx/AffineTransform.h>
#include <LibWeb/Geometry/DOMPoint.h>
#include <LibWeb/SVG/AttributeParser.h>
#include <LibWeb/SVG/SVGGraphicsElement.h>

namespace Web::SVG {

// https://svgwg.org/svg2-draft/types.html#InterfaceSVGGeometryElement
class SVGGeometryElement : public SVGGraphicsElement {
    WEB_PLATFORM_OBJECT(SVGGeometryElement, SVGGraphicsElement);

public:
    virtual JS::GCPtr<Layout::Node> create_layout_node(NonnullRefPtr<CSS::StyleProperties>) override;

    virtual Gfx::Path& get_path() = 0;

    virtual void parse_attribute(DeprecatedFlyString const& name, DeprecatedString const& value) override;

    Gfx::AffineTransform get_transform() const;

    float get_total_length();
    JS::NonnullGCPtr<Geometry::DOMPoint> get_point_at_length(float distance);

protected:
    SVGGeometryElement(DOM::Document& document, DOM::QualifiedName qualified_name);

    virtual JS::ThrowCompletionOr<void> initialize(JS::Realm&) override;

private:
    Gfx::AffineTransform m_transform {};
};

Gfx::AffineTransform transform_from_transform_list(ReadonlySpan<Transform> tranform_list);

}
