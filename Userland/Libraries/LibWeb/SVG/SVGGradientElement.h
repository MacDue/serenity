/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibGfx/PaintStyle.h>
#include <LibWeb/SVG/AttributeParser.h>
#include <LibWeb/SVG/SVGElement.h>

namespace Web::SVG {

class SVGGradientElement : public SVGElement {
    WEB_PLATFORM_OBJECT(SVGGradientElement, SVGElement);

public:
    virtual ~SVGGradientElement() override = default;

    virtual void parse_attribute(DeprecatedFlyString const& name, DeprecatedString const& value) override;

    virtual Optional<Gfx::PaintStyle const&> to_gfx_paint_style(float viewbox_scale) const = 0;

    GradientUnits gradient_units() const
    {
        return m_gradient_units.value_or(GradientUnits::ObjectBoundingBox);
    }

    Optional<Gfx::AffineTransform> const& gradient_transform() const
    {
        return m_gradient_transform;
    }

protected:
    SVGGradientElement(DOM::Document&, DOM::QualifiedName);

    virtual JS::ThrowCompletionOr<void> initialize(JS::Realm&) override;

private:
    Optional<GradientUnits> m_gradient_units = {};
    Optional<Gfx::AffineTransform> m_gradient_transform = {};
};

}
