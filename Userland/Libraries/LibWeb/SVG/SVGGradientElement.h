/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibGfx/PaintStyle.h>
#include <LibWeb/SVG/SVGElement.h>

namespace Web::SVG {

class SVGGradientElement : public SVGElement {
    WEB_PLATFORM_OBJECT(SVGGradientElement, SVGElement);

public:
    virtual ~SVGGradientElement() override = default;

    virtual Gfx::PaintStyle const& to_gfx_paint_style() const = 0;

protected:
    SVGGradientElement(DOM::Document&, DOM::QualifiedName);

    virtual JS::ThrowCompletionOr<void> initialize(JS::Realm&) override;
};

}
