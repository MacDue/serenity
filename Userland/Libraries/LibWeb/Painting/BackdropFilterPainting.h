/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibGfx/Forward.h>
#include <LibWeb/CSS/BackdropFilter.h>
#include <LibWeb/Forward.h>

namespace Web::Painting {

void apply_backdrop_filter(PaintContext&, Layout::Node const&, Gfx::FloatRect const&, BorderRadiiData const&, CSS::BackdropFilter const&);

}
