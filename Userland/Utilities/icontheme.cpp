

#include <LibMain/Main.h>
#include <AK/String.h>
#include <LibCore/ArgsParser.h>
#include <LibCore/EventLoop.h>
#include <LibResource/PathResolverClient.h>


ErrorOr<int> serenity_main(Main::Arguments args)
{
    Core::EventLoop event_loop;
    TRY(Core::System::pledge("unix"));
    TRY(Core::System::unveil("/tmp/portal/resource", "rw"));
    TRY(Core::System::unveil(nullptr, nullptr));

    bool remove = false;
    String theme;
    Core::ArgsParser args_parser;
    args_parser.add_positional_argument(theme, "Icon theme to set", "theme");
    args_parser.add_option(remove, "remove", nullptr, 'r');
    args_parser.parse(args);

    if (theme.is_empty())
        return 1;

    auto client = TRY(Resource::PathResolverClient::try_create());
    auto theme_path = String::formatted("/res/icon-themes/{}", theme);
    if (!remove)
        client->add_resource_path(theme_path);
    else
        client->remove_resource_path(theme_path);

    dbgln("Ok!");
    return 0;
}
