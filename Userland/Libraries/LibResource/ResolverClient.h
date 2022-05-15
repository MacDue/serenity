/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Function.h>
#include <LibIPC/ConnectionToServer.h>
#include <ResourceServer/ResourceClientEndpoint.h>
#include <ResourceServer/ResourceServerEndpoint.h>

namespace Resource {

using namespace ResourceServer;

class ResolverClient final
    : public IPC::ConnectionToServer<ResourceClientEndpoint, ResourceServerEndpoint>
    , public LaunchClientEndpoint {
    IPC_CLIENT_CONNECTION(ResolverClient, "/tmp/portal/resource")
private:
    ResolverClient(NonnullOwnPtr<Core::Stream::LocalSocket> socket)
        : IPC::ConnectionToServer<ResourceClientEndpoint, ResourceServerEndpoint>(*this, move(socket))
        , m_event_target { event_target }
    {
    }

    virtual void resource_paths_updated() override
    {
        if (on_resource_paths_updated)
            on_resource_paths_updated();
    }

public:
    static ResolverClient& the();

    Function<void()> on_resource_paths_updated;
};

}
