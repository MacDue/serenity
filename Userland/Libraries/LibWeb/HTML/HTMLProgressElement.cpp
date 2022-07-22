/*
 * Copyright (c) 2020-2022, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/DOM/Document.h>
#include <LibWeb/DOM/ShadowRoot.h>
#include <LibWeb/HTML/HTMLProgressElement.h>
#include <LibWeb/Layout/BlockContainer.h>
#include <LibWeb/Layout/Node.h>
#include <stdlib.h>

namespace Web::HTML {

HTMLProgressElement::HTMLProgressElement(DOM::Document& document, DOM::QualifiedName qualified_name)
    : HTMLElement(document, move(qualified_name))
{
}

HTMLProgressElement::~HTMLProgressElement() = default;

RefPtr<Layout::Node> HTMLProgressElement::create_layout_node(NonnullRefPtr<CSS::StyleProperties> style)
{
    auto layout_node = adopt_ref(*new Layout::BlockContainer(document(), this, move(style)));
    layout_node->set_inline(true);
    return layout_node;
}

double HTMLProgressElement::value() const
{
    auto value_characters = attribute(HTML::AttributeNames::value).characters();
    if (value_characters == nullptr)
        return 0;

    auto parsed_value = strtod(value_characters, nullptr);
    if (!isfinite(parsed_value) || parsed_value < 0)
        return 0;

    return min(parsed_value, max());
}

void HTMLProgressElement::set_value(double value)
{
    if (value < 0)
        return;

    set_attribute(HTML::AttributeNames::value, String::number(value));
    document().invalidate_layout();
}

double HTMLProgressElement::max() const
{
    auto max_characters = attribute(HTML::AttributeNames::max).characters();
    if (max_characters == nullptr)
        return 1;

    auto parsed_value = strtod(max_characters, nullptr);
    if (!isfinite(parsed_value) || parsed_value <= 0)
        return 1;

    return parsed_value;
}

void HTMLProgressElement::set_max(double value)
{
    if (value <= 0)
        return;

    set_attribute(HTML::AttributeNames::max, String::number(value));
    document().invalidate_layout();
}

double HTMLProgressElement::position() const
{
    if (!is_determinate())
        return -1;

    return value() / max();
}

}
