/*
 * Copyright (c) 2024, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/Layout/SVGMaskBox.h>
#include <LibWeb/Painting/SVGGraphicsPaintable.h>

namespace Web::Painting {

class SVGMaskPaintable : public SVGGraphicsPaintable {
    JS_CELL(SVGMaskPaintable, SVGGraphicsPaintable);

public:
    static JS::NonnullGCPtr<SVGMaskPaintable> create(Layout::SVGMaskBox const&);

protected:
    SVGMaskPaintable(Layout::SVGMaskBox const&);
};

}
