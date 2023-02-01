/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibGfx/PaintStyle.h>
#include <LibWeb/Bindings/PlatformObject.h>
#include <LibWeb/HTML/Canvas/CanvasDrawImage.h>

namespace Web::HTML {

class CanvasPattern final : public Bindings::PlatformObject {
    WEB_PLATFORM_OBJECT(CanvasPattern, Bindings::PlatformObject);

public:
    static WebIDL::ExceptionOr<JS::GCPtr<CanvasPattern>> create(JS::Realm&, CanvasImageSource const& image, StringView repetition)
    {
        (void)image;
        (void)repetition;
        VERIFY_NOT_REACHED();
    }

    ~CanvasPattern();

    NonnullRefPtr<Gfx::PaintStyle> to_gfx_paint_style() { VERIFY_NOT_REACHED(); }

private:
    virtual JS::ThrowCompletionOr<void> initialize(JS::Realm&) override;
};

}
