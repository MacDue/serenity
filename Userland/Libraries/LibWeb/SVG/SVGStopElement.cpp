/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/SVG/SVGStopElement.h>

namespace Web::SVG {

SVGStopElement::SVGStopElement(DOM::Document& document, DOM::QualifiedName qualified_name)
    : SVGElement(document, qualified_name)
{
}

void SVGStopElement::parse_attribute(DeprecatedFlyString const& name, DeprecatedString const& value)
{
    SVGElement::parse_attribute(name, value);
    // TODO: Parse offset
}

JS::NonnullGCPtr<SVGAnimatedLength> SVGStopElement::offset() const
{
    VERIFY_NOT_REACHED();
}

JS::ThrowCompletionOr<void> SVGStopElement::initialize(JS::Realm& realm)
{
    MUST_OR_THROW_OOM(Base::initialize(realm));
    set_prototype(&Bindings::ensure_web_prototype<Bindings::SVGStopElementPrototype>(realm, "SVGStopElement"));

    return {};
}

}
