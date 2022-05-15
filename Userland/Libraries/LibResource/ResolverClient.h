/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Function.h>
#include <AK/HashMap.h>
#include <AK/IDAllocator.h>
#include <LibGfx/Bitmap.h>
#include <LibIPC/ConnectionToServer.h>
#include <ResourceServer/ResourceClientEndpoint.h>
#include <ResourceServer/ResourceServerEndpoint.h>

namespace Resource {

class PathWatcher;

class ResolverClient final
    : public IPC::ConnectionToServer<ResourceClientEndpoint, ResourceServerEndpoint>
    , public ResourceClientEndpoint {
    IPC_CLIENT_CONNECTION(ResolverClient, "/tmp/portal/resource")
public:
    friend PathWatcher;
    static ResolverClient& the();

    ErrorOr<NonnullRefPtr<Gfx::Bitmap>> try_resolve_and_load_bitmap(String const& path)
    {
        return Gfx::Bitmap::try_load_from_file(resolve(path));
    }

private:
    i32 add_path_watcher(PathWatcher& path_watchers);
    void remove_path_watcher(i32 id);

    ResolverClient(NonnullOwnPtr<Core::Stream::LocalSocket> socket)
        : IPC::ConnectionToServer<ResourceClientEndpoint, ResourceServerEndpoint>(*this, move(socket))
    {
    }

    virtual void resource_paths_updated() override;

    IDAllocator m_id_allocator;
    HashMap<i32, PathWatcher*> m_path_watchers;
};

class PathWatcher final {
public:
    using PathsUpdatedCallback = Function<void()>;

    PathWatcher()
    {
        m_id = ResolverClient::the().add_path_watcher(*this);
    }

    ~PathWatcher()
    {
        ResolverClient::the().remove_path_watcher(m_id);
    }

    void notify()
    {
        if (on_resource_paths_updated)
            on_resource_paths_updated();
    }

    PathsUpdatedCallback on_resource_paths_updated;

    AK_MAKE_NONCOPYABLE(PathWatcher);
    AK_MAKE_NONMOVABLE(PathWatcher);

private:
    i32 m_id;
};

}
