/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/HashMap.h>
#include <ResourceServer/ConnectionFromClient.h>
#include <ResourceServer/Resolver.h>

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
    return Resolver::the().resolve(partial_path);
}

Messages::ResourceServer::AddResourcePathResponse ConnectionFromClient::add_resource_path(const AK::String& resource_path)
{
    return !Resolver::the().add_resource_path(resource_path).is_error();
}

Messages::ResourceServer::RemoveResourcePathResponse ConnectionFromClient::remove_resource_path(const AK::String& resource_path)
{
    return Resolver::the().remove_resource_path(resource_path);
}

}
