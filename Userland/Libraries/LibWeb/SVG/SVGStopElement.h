/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/SVG/SVGAnimatedNumber.h>
#include <LibWeb/SVG/SVGElement.h>

namespace Web::SVG {

class SVGStopElement final : public SVGElement {
    WEB_PLATFORM_OBJECT(SVGStopElement, SVGElement);

public:
    virtual ~SVGStopElement() override = default;

    virtual void parse_attribute(DeprecatedFlyString const& name, DeprecatedString const& value) override;

    JS::NonnullGCPtr<SVGAnimatedNumber> offset() const;

    Optional<float> stop_offset() const { return m_offset; }
    Optional<Gfx::Color> stop_color() const;

private:
    SVGStopElement(DOM::Document&, DOM::QualifiedName);

    virtual JS::ThrowCompletionOr<void> initialize(JS::Realm&) override;

    Optional<float> m_offset;
    Optional<Gfx::Color> m_color;
};

}
