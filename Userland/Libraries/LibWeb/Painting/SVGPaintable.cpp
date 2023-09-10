/*
 * Copyright (c) 2018-2022, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Layout/ImageBox.h>
#include <LibWeb/Layout/SVGSVGBox.h>
#include <LibWeb/Painting/SVGPaintable.h>
#include <LibWeb/Painting/StackingContext.h>
#include <LibWeb/SVG/SVGMaskElement.h>

namespace Web::Painting {

SVGPaintable::SVGPaintable(Layout::SVGBox const& layout_box)
    : PaintableBox(layout_box)
{
}

Layout::SVGBox const& SVGPaintable::layout_box() const
{
    return static_cast<Layout::SVGBox const&>(layout_node());
}

CSSPixelRect SVGPaintable::compute_absolute_rect() const
{
    if (auto* svg_svg_box = layout_box().first_ancestor_of_type<Layout::SVGSVGBox>()) {
        CSSPixelRect rect { offset(), content_size() };
        for (Layout::Box const* ancestor = svg_svg_box; ancestor && ancestor->paintable(); ancestor = ancestor->paintable()->containing_block())
            rect.translate_by(ancestor->paintable_box()->offset());
        return rect;
    }
    return PaintableBox::compute_absolute_rect();
}

CSSPixelRect SVGPaintable::get_masking_area() const
{
    if (!is<SVG::SVGGraphicsElement>(dom_node()))
        return {};
    auto const& graphics_element = static_cast<SVG::SVGGraphicsElement const&>(*dom_node());
    return graphics_element.mask()->resolve_masking_area(absolute_border_box_rect());
}

void SVGPaintable::apply_mask(PaintContext& context, Gfx::Bitmap& target, CSSPixelRect const& masking_area) const
{
    if (!is<SVG::SVGGraphicsElement>(dom_node()))
        return;
    auto const& graphics_element = static_cast<SVG::SVGGraphicsElement const&>(*dom_node());
    auto mask = graphics_element.mask();
    if (mask->mask_content_units() != SVG::MaskContentUnits::UserSpaceOnUse) {
        dbgln("SVG: maskContentUnits=objectBoundingBox is not supported");
        return;
    }
    auto mask_rect = context.enclosing_device_rect(masking_area);
    RefPtr<Gfx::Bitmap> mask_bitmap = {};
    if (mask && mask->layout_node() && is<PaintableBox>(mask->layout_node()->paintable())) {
        auto& mask_paintable = static_cast<PaintableBox const&>(*mask->layout_node()->paintable());
        auto mask_bitmap_or_error = Gfx::Bitmap::create(Gfx::BitmapFormat::BGRA8888, mask_rect.size().to_type<int>());
        if (mask_bitmap_or_error.is_error())
            return;
        mask_bitmap = mask_bitmap_or_error.release_value();
        {
            Gfx::Painter painter(*mask_bitmap);
            painter.translate(-mask_rect.location().to_type<int>());
            auto paint_context = context.clone(painter);
            paint_context.set_svg_mask_painting(true);
            paint_context.set_svg_transform(graphics_element.get_transform());
            StackingContext::paint_node_as_stacking_context(mask_paintable, paint_context);
        }
    }
    // TODO: Follow mask-type attribute to select between alpha/luminance masks.
    if (mask_bitmap)
        target.apply_mask(*mask_bitmap, Gfx::Bitmap::MaskKind::Luminance);
    return;
}

}
