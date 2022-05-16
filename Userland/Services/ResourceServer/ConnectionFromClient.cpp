/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/HashMap.h>
#include <ResourceServer/ConnectionFromClient.h>
#include <ResourceServer/PathResolver.h>

namespace ResourceServer {

static HashMap<int, RefPtr<ConnectionFromClient>> s_connections;
ConnectionFromClient::ConnectionFromClient(NonnullOwnPtr<Core::Stream::LocalSocket> client_socket, int client_id)
    : IPC::ConnectionFromClient<ResourceClientEndpoint, ResourceServerEndpoint>(*this, move(client_socket), client_id)
{
    s_connections.set(client_id, *this);
}

void ConnectionFromClient::die()
{
    s_connections.remove(client_id());
}

Messages::ResourceServer::ResolveResponse ConnectionFromClient::resolve(const AK::String& partial_path)
{
    return PathResolver::the().resolve(partial_path);
}

Messages::ResourceServer::AddResourcePathResponse ConnectionFromClient::add_resource_path(const AK::String& resource_path)
{
    if (!PathResolver::the().add_resource_path(resource_path).is_error()) {
        notify_resource_paths_updated();
        return true;
    }
    return false;
}

Messages::ResourceServer::RemoveResourcePathResponse ConnectionFromClient::remove_resource_path(const AK::String& resource_path)
{
    if (PathResolver::the().remove_resource_path(resource_path)) {
        notify_resource_paths_updated();
        return true;
    }
    return false;
}

void ConnectionFromClient::notify_resource_paths_updated()
{
    for (auto& [_, client] : s_connections)
        client->async_resource_paths_updated();
}

}
