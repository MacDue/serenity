/*
 * Copyright (c) 2022, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibGfx/FillStyle.h>
#include <LibWeb/Bindings/PlatformObject.h>

namespace Web::HTML {

class CanvasGradient final : public Bindings::PlatformObject {
    WEB_PLATFORM_OBJECT(CanvasGradient, Bindings::PlatformObject);

public:
    static JS::NonnullGCPtr<CanvasGradient> create_radial(JS::Realm&, double x0, double y0, double r0, double x1, double y1, double r1);
    static JS::NonnullGCPtr<CanvasGradient> create_linear(JS::Realm&, double x0, double y0, double x1, double y1);
    static JS::NonnullGCPtr<CanvasGradient> create_conic(JS::Realm&, double start_angle, double x, double y);

    WebIDL::ExceptionOr<void> add_color_stop(double offset, DeprecatedString const& color);

    ~CanvasGradient();

    NonnullRefPtr<Gfx::FillStyle> to_gfx_fill_style() { return m_gradient_fill; }

private:
    CanvasGradient(JS::Realm&, Gfx::GradientFillStyle& gradient_fill);

    virtual void initialize(JS::Realm&) override;

    NonnullRefPtr<Gfx::GradientFillStyle> m_gradient_fill;
};

}
