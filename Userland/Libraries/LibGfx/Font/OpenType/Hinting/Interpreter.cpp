/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Font/OpenType/Hinting/Interpreter.h>

namespace OpenType::Hinting {

ErrorOr<Interpreter> Interpreter::create(Font& font)
{
    // auto maxp = font.m_maxp.header();
}

}
