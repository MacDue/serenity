/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/HTML/CanvasPattern.h>

namespace Web::HTML {

CanvasPattern::~CanvasPattern() = default;

JS::ThrowCompletionOr<void> CanvasPattern::initialize(JS::Realm& realm)
{
    MUST_OR_THROW_OOM(Base::initialize(realm));
    set_prototype(&Bindings::ensure_web_prototype<Bindings::CanvasPatternPrototype>(realm, "CanvasPattern"));

    return {};
}

}
