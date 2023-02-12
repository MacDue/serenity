/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Format.h>
#include <LibCore/ArgsParser.h>
#include <LibGfx/Font/OpenType/Font.h>
#include <LibGfx/Font/OpenType/Hinting/Interpreter.h>

using namespace OpenType::Hinting;

ErrorOr<int> serenity_main(Main::Arguments arguments)
{
    Core::ArgsParser args_parser;

    StringView font_path;
    args_parser.add_positional_argument(font_path, "Path to font", "FILE");
    args_parser.parse(arguments);

    auto font = TRY(OpenType::Font::try_load_from_file(font_path));
    auto interpreter = TRY(Interpreter::create(font));
    auto instance = TRY(interpreter->create_font_instance_data(18));

    interpreter->execute_program(*font->font_program(), {});
    interpreter->execute_program(*font->control_value_program(), { *instance });
    return 0;
}
