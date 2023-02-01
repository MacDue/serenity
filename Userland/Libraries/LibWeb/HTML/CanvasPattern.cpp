/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Bitmap.h>
#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/HTML/CanvasPattern.h>

namespace Web::HTML {

Gfx::Color CanvasPatternPaintStyle::sample_color(Gfx::IntPoint) const
{
    return Gfx::Color {};
}

CanvasPattern::CanvasPattern(JS::Realm& realm, CanvasPatternPaintStyle& pattern)
    : PlatformObject(realm)
    , m_pattern(pattern)
{
}

CanvasPattern::~CanvasPattern() = default;

WebIDL::ExceptionOr<JS::GCPtr<CanvasPattern>> CanvasPattern::create(JS::Realm&, CanvasImageSource const&, StringView)
{
    VERIFY_NOT_REACHED();
}

JS::ThrowCompletionOr<void> CanvasPattern::initialize(JS::Realm& realm)
{
    MUST_OR_THROW_OOM(Base::initialize(realm));
    set_prototype(&Bindings::ensure_web_prototype<Bindings::CanvasPatternPrototype>(realm, "CanvasPattern"));

    return {};
}

}
