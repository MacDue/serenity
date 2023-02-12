/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Format.h>
#include <LibCore/ArgsParser.h>
#include <LibGfx/Font/OpenType/Font.h>
#include <LibGfx/Font/OpenType/Hinting/Interpreter.h>

ErrorOr<int> serenity_main(Main::Arguments arguments)
{
    Core::ArgsParser args_parser;

    StringView font_path;
    args_parser.add_positional_argument(font_path, "Path to font", "FILE");
    args_parser.parse(arguments);

    auto font = TRY(OpenType::Font::try_load_from_file(font_path));
    OpenType::Hinting::InstructionStream font_program { *font->font_program() };
    auto interpreter = TRY(OpenType::Hinting::Interpreter::create(font));
    interpreter.execute_program(font_program);

    return 0;
}
