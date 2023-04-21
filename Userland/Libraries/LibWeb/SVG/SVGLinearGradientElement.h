/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/SVG/SVGAnimatedLength.h>
#include <LibWeb/SVG/SVGGradientElement.h>

namespace Web::SVG {

class SVGLinearGradientElement : public SVGGradientElement {
    WEB_PLATFORM_OBJECT(SVGLinearGradientElement, SVGGradientElement);

public:
    virtual ~SVGLinearGradientElement() override = default;

    virtual void parse_attribute(DeprecatedFlyString const& name, DeprecatedString const& value) override;

    virtual Optional<Gfx::PaintStyle const&> to_gfx_paint_style(Gfx::AffineTransform const& transform) const override;

    JS::NonnullGCPtr<SVGAnimatedLength> x1() const;
    JS::NonnullGCPtr<SVGAnimatedLength> y1() const;
    JS::NonnullGCPtr<SVGAnimatedLength> x2() const;
    JS::NonnullGCPtr<SVGAnimatedLength> y2() const;

protected:
    SVGLinearGradientElement(DOM::Document&, DOM::QualifiedName);

    virtual JS::ThrowCompletionOr<void> initialize(JS::Realm&) override;

private:
    JS::GCPtr<SVGLinearGradientElement const> linear_gradient_xlink_href() const
    {
        if (auto href = xlink_href(); href && is<SVGLinearGradientElement>(*href))
            return &verify_cast<SVGLinearGradientElement>(*href);
        return {};
    }

    float start_x() const;
    float start_y() const;
    float end_x() const;
    float end_y() const;

    Optional<float> m_x1;
    Optional<float> m_y1;
    Optional<float> m_x2;
    Optional<float> m_y2;

    mutable RefPtr<Gfx::PaintStyle> m_paint_style;
};

}
