/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/CSS/Parser/Parser.h>
#include <LibWeb/CSS/StyleValues/IdentifierStyleValue.h>
#include <LibWeb/Layout/BlockContainer.h>
#include <LibWeb/SVG/AttributeNames.h>
#include <LibWeb/SVG/AttributeParser.h>
#include <LibWeb/SVG/SVGStopElement.h>

namespace Web::SVG {

SVGStopElement::SVGStopElement(DOM::Document& document, DOM::QualifiedName qualified_name)
    : SVGElement(document, qualified_name)
{
}

void SVGStopElement::parse_attribute(DeprecatedFlyString const& name, DeprecatedString const& value)
{
    SVGElement::parse_attribute(name, value);
    if (name == SVG::AttributeNames::offset) {
        m_offset = AttributeParser::parse_length_percentage(value);
        dbgln("<stop>: Parsed offset: {}", m_offset);
    } else if (name.equals_ignoring_ascii_case("stop-color"sv)) {
        CSS::Parser::ParsingContext parsing_context { document() };
        if (auto stop_color = parse_css_value(parsing_context, value, CSS::PropertyID::StopColor)) {
            // HACK!!!! Pass null to_color() since this never has a layout node!
            m_color = stop_color->to_color(*layout_node());
        }
        dbgln("<stop>: Parsed color: {}", m_color);
    }
}

// void SVGStopElement::apply_presentational_hints(CSS::StyleProperties&) const
// {
// }

Optional<Gfx::Color> SVGStopElement::stop_color() const
{
    return m_color;
}

JS::NonnullGCPtr<SVGAnimatedNumber> SVGStopElement::offset() const
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
