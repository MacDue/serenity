/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibIPC/ConnectionFromClient.h>
#include <ResourceServer/ResourceClientEndpoint.h>
#include <ResourceServer/ResourceServerEndpoint.h>

namespace ResourceServer {

class ConnectionFromClient final : public IPC::ConnectionFromClient<ResourceClientEndpoint, ResourceServerEndpoint> {
    C_OBJECT(ConnectionFromClient)
public:
    ~ConnectionFromClient() override = default;

    virtual void die() override;

    virtual Messages::ResourceServer::ResolveResponse resolve(const AK::String&) override;

    virtual Messages::ResourceServer::AddResourcePathResponse add_resource_path(const AK::String&) override;
    virtual Messages::ResourceServer::RemoveResourcePathResponse remove_resource_path(const AK::String&) override;

private:
    explicit ConnectionFromClient(NonnullOwnPtr<Core::Stream::LocalSocket>, int client_id);
};
}
