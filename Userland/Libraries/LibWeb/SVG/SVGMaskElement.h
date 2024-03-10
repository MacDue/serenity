/*
 * Copyright (c) 2023, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/SVG/AttributeParser.h>
#include <LibWeb/SVG/SVGGraphicsElement.h>
#include <LibWeb/SVG/SVGViewport.h>

namespace Web::SVG {

class SVGMaskElement final : public SVGGraphicsElement
    , public SVGViewport {

    WEB_PLATFORM_OBJECT(SVGMaskElement, SVGGraphicsElement);
    JS_DECLARE_ALLOCATOR(SVGMaskElement);

public:
    virtual ~SVGMaskElement() override;

    virtual Optional<ViewBox> view_box() const override
    {
        if (mask_content_units() == MaskContentUnits::ObjectBoundingBox)
            return ViewBox { 0, 0, 1, 1 };
        return {};
    }
    virtual Optional<PreserveAspectRatio> preserve_aspect_ratio() const override
    {
        return PreserveAspectRatio { PreserveAspectRatio::Align::None, PreserveAspectRatio::MeetOrSlice::Meet };
    }

    virtual void attribute_changed(FlyString const& name, Optional<String> const& value) override;

    virtual JS::GCPtr<Layout::Node> create_layout_node(NonnullRefPtr<CSS::StyleProperties>) override;

    CSSPixelRect resolve_masking_area(CSSPixelRect const& mask_target) const;

    MaskContentUnits mask_content_units() const;
    MaskUnits mask_units() const;

private:
    SVGMaskElement(DOM::Document&, DOM::QualifiedName);
    virtual void initialize(JS::Realm&) override;

    Optional<MaskContentUnits> m_mask_content_units = {};
    Optional<MaskUnits> m_mask_units = {};
};

}
