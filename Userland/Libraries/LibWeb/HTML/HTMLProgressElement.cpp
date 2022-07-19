/*
 * Copyright (c) 2020-2022, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/HTML/HTMLProgressElement.h>
#include <LibWeb/Layout/Progress.h>
#include <LibWeb/DOM/ShadowRoot.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/Layout/BlockContainer.h>
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
    update_value();
}

void HTMLProgressElement::update_value() {
    if (m_progress_value) {
        auto style = m_progress_value->style_for_bindings();
        auto current_position = position();
        style->set_property(CSS::PropertyID::Width, current_position >= 0
            ? String::formatted("{}%", current_position * 100) : "0px");
    }
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
    update_value();
}

double HTMLProgressElement::position() const
{
    if (!is_determinate())
        return -1;

    return value() / max();
}

void HTMLProgressElement::inserted() {
    create_shadow_tree_if_needed();
}

void HTMLProgressElement::create_shadow_tree_if_needed() {
    if (shadow_root())
        return;
    auto shadow_root = adopt_ref(*new DOM::ShadowRoot(document(), *this));
    auto element = document().create_element(HTML::TagNames::div).release_value();
    m_progress_value = document().create_element(HTML::TagNames::div).release_value();
    element->set_attribute(HTML::AttributeNames::style, "margin: 1px; height: 15px; width: 200px; border: 1px solid black; background-color: white;");
    m_progress_value->set_attribute(HTML::AttributeNames::style, "height: 100%; background-color: green;");
    update_value();
    element->append_child(*m_progress_value);
    shadow_root->append_child(move(element));
    set_shadow_root(move(shadow_root));
}

}
