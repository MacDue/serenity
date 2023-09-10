/*
 * Copyright (c) 2022, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/Layout/SVGBox.h>
#include <LibWeb/Painting/PaintableBox.h>

namespace Web::Painting {

class SVGPaintable : public PaintableBox {
    JS_CELL(SVGPaintable, PaintableBox);

public:
    Layout::SVGBox const& layout_box() const;

    virtual CSSPixelRect get_masking_area() const override;
    virtual void apply_mask(PaintContext&, Gfx::Bitmap& target, CSSPixelRect const& masking_area) const override;

protected:
    SVGPaintable(Layout::SVGBox const&);

    virtual CSSPixelRect compute_absolute_rect() const override;
};

}
