/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Function.h>
#include <AK/String.h>
#include <AK/StringView.h>
#include <AK/Vector.h>

namespace ResourceServer {

class PathResolver {
public:
    static PathResolver& the();

    String resolve(StringView partial_path) const;
    ErrorOr<void> add_resource_path(String resource_path);
    bool remove_resource_path(String resource_path);

    ErrorOr<void> try_load_resouce_paths();

    ErrorOr<void> save_resource_paths();

    Function<void()> on_resource_paths_changed;

    void notify_resource_paths_changed()
    {
        if (on_resource_paths_changed)
            on_resource_paths_changed();
    }

private:
    Vector<String> m_resource_paths;
};

}
