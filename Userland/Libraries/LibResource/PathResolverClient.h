#pragma once

#include <LibIPC/ConnectionToServer.h>
#include <ResourceServer/ResourceClientEndpoint.h>
#include <ResourceServer/ResourceServerEndpoint.h>

namespace Resource {

class PathResolverClient final
    : public IPC::ConnectionToServer<ResourceClientEndpoint, ResourceServerEndpoint>
    , public ResourceClientEndpoint {
    IPC_CLIENT_CONNECTION(PathResolverClient, "/tmp/portal/resource")
public:
private:
    PathResolverClient(NonnullOwnPtr<Core::Stream::LocalSocket> socket)
        : IPC::ConnectionToServer<ResourceClientEndpoint, ResourceServerEndpoint>(*this, move(socket))
    {
    }

    virtual void resource_paths_updated() override
    {
        // TODO
    }

    virtual void die() override { }
};

String resolve_path(StringView partial_string, int keep_alive_ms = 2000);
}
