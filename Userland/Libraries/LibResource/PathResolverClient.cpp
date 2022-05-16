
#include <AK/Optional.h>
#include <LibCore/Timer.h>
#include <LibResource/PathResolverClient.h>

namespace Resource {
String resolve_path(StringView partial_string, int keep_alive_ms)
{
    static RefPtr<PathResolverClient> client;
    static auto connection_close_timeout = Core::Timer::create_single_shot(keep_alive_ms, [&] {
        client = nullptr;
    });
    connection_close_timeout->stop();
    if (!client)
        client = PathResolverClient::try_create().release_value_but_fixme_should_propagate_errors();
    connection_close_timeout->set_interval(keep_alive_ms);
    connection_close_timeout->start();
    return client->resolve(partial_string);
}
}
