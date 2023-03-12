/*
 * Copyright (c) 2020, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/AreoWindowTheme.h>
#include <LibGfx/ClassicWindowTheme.h>
#include <LibGfx/LunaWindowTheme.h>
#include <LibGfx/WindowTheme.h>

namespace Gfx {

WindowTheme& WindowTheme::current()
{
    static LunaWindowTheme theme;
    return theme;
}

}
