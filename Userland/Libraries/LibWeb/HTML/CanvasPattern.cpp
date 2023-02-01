/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Bitmap.h>
#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/HTML/CanvasPattern.h>
#include <LibWeb/HTML/CanvasRenderingContext2D.h>

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

// https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-createpattern
WebIDL::ExceptionOr<JS::GCPtr<CanvasPattern>> CanvasPattern::create(JS::Realm& realm, CanvasImageSource const& image, StringView repetition)
{
    auto parse_repetition = [&](auto repetition) -> Optional<CanvasPatternPaintStyle::Repetition> {
        if (repetition == "repeat"sv)
            return CanvasPatternPaintStyle::Repetition::Repeat;
        if (repetition == "repeat-x"sv)
            return CanvasPatternPaintStyle::Repetition::RepeatX;
        if (repetition == "repeat-y"sv)
            return CanvasPatternPaintStyle::Repetition::RepeatY;
        if (repetition == "no-repeat"sv)
            return CanvasPatternPaintStyle::Repetition::NoRepeat;
        return {};
    };

    // 1. Let usability be the result of checking the usability of image.
    auto usability = TRY(check_usability_of_image(image));

    // 2. If usability is bad, then return null.
    if (usability == CanvasImageSourceUsability::Bad)
        return JS::GCPtr<CanvasPattern> {};

    // 3. Assert: usability is good.
    VERIFY(usability == CanvasImageSourceUsability::Good);

    // 4. If repetition is the empty string, then set it to "repeat".
    if (repetition.is_empty())
        repetition = "repeat"sv;

    // 5. If repetition is not identical to one of "repeat", "repeat-x", "repeat-y", or "no-repeat",
    // then throw a "SyntaxError" DOMException.
    auto repetition_value = parse_repetition(repetition);
    if (!repetition_value.has_value())
        return WebIDL::SyntaxError::create(realm, "Repetition value is not valid");

    // Note: Bitmap won't be null here, as if it were it would have "bad" usability.
    auto const& bitmap = *image.visit([](auto const& source) -> Gfx::Bitmap const* { return source->bitmap(); });

    // 6. Let pattern be a new CanvasPattern object with the image image and the repetition behavior given by repetition.
    auto pattern = CanvasPatternPaintStyle::create(bitmap, *repetition_value);

    // FIXME: 7. If image is not origin-clean, then mark pattern as not origin-clean.

    // 8. Return pattern.
    return MUST_OR_THROW_OOM(realm.heap().allocate<CanvasPattern>(realm, realm, *pattern));
}

JS::ThrowCompletionOr<void> CanvasPattern::initialize(JS::Realm& realm)
{
    MUST_OR_THROW_OOM(Base::initialize(realm));
    set_prototype(&Bindings::ensure_web_prototype<Bindings::CanvasPatternPrototype>(realm, "CanvasPattern"));

    return {};
}

}
