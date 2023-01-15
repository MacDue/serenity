/*
 * Copyright (c) 2022, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/QuickSort.h>
#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/HTML/CanvasGradient.h>
#include <LibWeb/WebIDL/ExceptionOr.h>

namespace Web::HTML {

JS::NonnullGCPtr<CanvasGradient> CanvasGradient::create_radial(JS::Realm& realm, double x0, double y0, double r0, double x1, double y1, double r1)
{
    (void)x0;
    (void)y0;
    (void)r0;
    (void)x1;
    (void)y1;
    (void)r1;
    auto placeholder = Gfx::SolidFillStyle::create(Gfx::Color::Red);
    return realm.heap().allocate<CanvasGradient>(realm, realm, placeholder);
}

JS::NonnullGCPtr<CanvasGradient> CanvasGradient::create_linear(JS::Realm& realm, double x0, double y0, double x1, double y1)
{
    (void)x0;
    (void)y0;
    (void)x1;
    (void)y1;
    auto placeholder = Gfx::SolidFillStyle::create(Gfx::Color::Red);
    return realm.heap().allocate<CanvasGradient>(realm, realm, placeholder);
}

JS::NonnullGCPtr<CanvasGradient> CanvasGradient::create_conic(JS::Realm& realm, double start_angle, double x, double y)
{
    auto conic_gradient = Gfx::ConicGradientFillStyle::create(Gfx::FloatPoint { x, y }.to_rounded<int>(), start_angle + 90.0);
    return realm.heap().allocate<CanvasGradient>(realm, realm, conic_gradient);
}

CanvasGradient::CanvasGradient(JS::Realm& realm, NonnullRefPtr<Gfx::FillStyle> gradient_fill)
    : PlatformObject(realm)
    , m_gradient_fill(gradient_fill)
{
}

CanvasGradient::~CanvasGradient() = default;

void CanvasGradient::initialize(JS::Realm& realm)
{
    Base::initialize(realm);
    set_prototype(&Bindings::ensure_web_prototype<Bindings::CanvasGradientPrototype>(realm, "CanvasGradient"));
}

// https://html.spec.whatwg.org/multipage/canvas.html#dom-canvasgradient-addcolorstop
WebIDL::ExceptionOr<void> CanvasGradient::add_color_stop(double offset, DeprecatedString const& color)
{
    // TODO: Remove once all gradient types are supported and placeholder fills can be removed.
    auto gradient_fill = dynamic_cast<Gfx::GradientFillStyle*>(m_gradient_fill.ptr());
    if (!gradient_fill)
        return {};

    // 1. If the offset is less than 0 or greater than 1, then throw an "IndexSizeError" DOMException.
    if (offset < 0 || offset > 1)
        return WebIDL::IndexSizeError::create(realm(), "CanvasGradient color stop offset out of bounds");

    // 2. Let parsed color be the result of parsing color.
    auto parsed_color = Color::from_string(color);

    // 3. If parsed color is failure, throw a "SyntaxError" DOMException.
    if (!parsed_color.has_value())
        return WebIDL::SyntaxError::create(realm(), "Could not parse color for CanvasGradient");

    // 4. Place a new stop on the gradient, at offset offset relative to the whole gradient, and with the color parsed color.
    MUST(gradient_fill->add_color_stop(offset, parsed_color.value()));

    // FIXME: If multiple stops are added at the same offset on a gradient, then they must be placed in the order added,
    //        with the first one closest to the start of the gradient, and each subsequent one infinitesimally further along
    //        towards the end point (in effect causing all but the first and last stop added at each point to be ignored).

    return {};
}

}
