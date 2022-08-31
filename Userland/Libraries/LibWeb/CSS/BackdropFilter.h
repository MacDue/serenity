/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Variant.h>
#include <LibWeb/CSS/StyleValue.h>

namespace Web::CSS {

class BackdropFilter {
public:
    BackdropFilter() = default;
    BackdropFilter(Vector<FilterFunction> filter_value_list)
        : m_filter_value_list(move(filter_value_list)) {};

    static inline BackdropFilter make_none()
    {
        return BackdropFilter {};
    }

    bool has_filters() const { return m_filter_value_list.has_value() && m_filter_value_list->size() > 0; }
    bool is_none() const { return !has_filters(); }

    Span<FilterFunction const> filters() const
    {
        VERIFY(!is_none());
        return m_filter_value_list->span();
    }

private:
    Optional<Vector<FilterFunction>> m_filter_value_list;
};

}
