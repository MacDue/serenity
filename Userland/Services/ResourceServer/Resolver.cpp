/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Format.h>
#include <AK/LexicalPath.h>
#include <LibCore/EventLoop.h>
#include <LibCore/File.h>
#include <LibCore/StandardPaths.h>
#include <LibCore/Stream.h>
#include <ResourceServer/Resolver.h>

namespace ResourceServer {

Resolver& Resolver::the()
{
    static Resolver the_resolver;
    return the_resolver;
}

String Resolver::resolve(StringView partial_path) const
{
    if (partial_path.starts_with('/'))
        return partial_path;
    for (auto const& resource_path : m_resource_paths.in_reverse()) {
        auto possible_path = LexicalPath::join(resource_path, partial_path).string();
        if (Core::File::exists(possible_path))
            return possible_path;
    }
    warnln("Failed to resolve path: {}", partial_path);
    // Default to resolving any failing paths to their /res equivalent
    return LexicalPath::join("/res", partial_path).string();
}

ErrorOr<void> Resolver::add_resource_path(String resource_path)
{
    if (m_resource_paths.find(resource_path) != m_resource_paths.end()) {
        return {};
    }
    TRY(m_resource_paths.try_append(move(resource_path)));
    notify_resource_paths_changed();
    return {};
}

bool Resolver::remove_resource_path(String resource_path)
{
    bool removed = m_resource_paths.remove_first_matching([&](auto& path) {
        return path == resource_path;
    });
    if (removed)
        notify_resource_paths_changed();
    return removed;
}

ErrorOr<void> Resolver::try_load_resouce_paths()
{
    auto file = TRY(Core::Stream::File::open(String::formatted("{}/ResourcePaths.txt", Core::StandardPaths::config_directory()), Core::Stream::OpenMode::Read));
    auto resource_paths = TRY(Core::Stream::BufferedFile::create(move(file)));
    auto buffer = TRY(ByteBuffer::create_uninitialized(256));
    while (TRY(resource_paths->can_read_line())) {
        auto line = TRY(resource_paths->read_line(buffer));
        if (!line.is_empty())
            TRY(add_resource_path(line));
    }
    return {};
}

ErrorOr<void> Resolver::save_resource_paths()
{
    // TODO
    return {};
}

}
