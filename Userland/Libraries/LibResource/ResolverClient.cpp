/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibResource/ResolverClient.h>

namespace Resource {

ResolverClient& ResolverClient::the()
{
    static auto connection = ResolverClient::try_create().release_value_but_fixme_should_propagate_errors();
    return connection;
}

i32 ResolverClient::add_path_watcher(PathWatcher& path_watcher)
{
    auto id = m_id_allocator.allocate();
    m_path_watchers.set(id, &path_watcher);
    return id;
}

void ResolverClient::remove_path_watcher(i32 id)
{
    m_path_watchers.remove(id);
    m_id_allocator.deallocate(id);
}

void ResolverClient::resource_paths_updated()
{
    for (auto [_, path_watcher] : m_path_watchers)
        path_watcher->notify();
}

}
