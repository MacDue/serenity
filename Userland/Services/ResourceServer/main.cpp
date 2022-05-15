/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Format.h>
#include <LibCore/ConfigFile.h>
#include <LibCore/EventLoop.h>
#include <LibCore/System.h>
#include <LibIPC/MultiServer.h>
#include <LibMain/Main.h>
#include <ResourceServer/ConnectionFromClient.h>
#include <ResourceServer/Resolver.h>

ErrorOr<int> serenity_main(Main::Arguments)
{
    Core::EventLoop event_loop;
    auto server = TRY(IPC::MultiServer<ResourceServer::ConnectionFromClient>::try_create());

    auto& resolver = ResourceServer::Resolver::the();
    (void)resolver.try_load_resouce_paths();

    resolver.on_resource_paths_changed = [&] {
        event_loop.deferred_invoke([&] {
            auto result = resolver.save_resource_paths();
            if (result.is_error())
                warnln("Failed to save resource paths");
        });
    };

    TRY(Core::System::pledge("stdio accept rpath"));
    TRY(Core::System::unveil("/home/anon/.config", "rw"));
    TRY(Core::System::unveil(nullptr, nullptr));

    return event_loop.exec();
}
