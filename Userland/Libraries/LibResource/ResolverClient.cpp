#include <ResolverClient.h>

namespace Resource {

static ResolverClient& ResolverClient::the()
{
    static auto connection = ResolverClient::try_create().release_value_but_fixme_should_propagate_errors();
    return connection;
}

}
