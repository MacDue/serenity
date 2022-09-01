/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Filters/StackBlurFilter.h>
#include <LibWeb/Layout/Node.h>
#include <LibWeb/Painting/BackdropFilterPainting.h>
#include <LibWeb/Painting/BorderRadiusCornerClipper.h>

namespace Web::Painting {

void apply_backdrop_filter(PaintContext& context, Layout::Node const& node, Gfx::FloatRect const& backdrop_rect, BorderRadiiData const& border_radii_data, CSS::BackdropFilter const& backdrop_filter)
{
    auto int_rect = backdrop_rect.to_rounded<int>();
    ScopedCornerRadiusClip corner_clipper { context.painter(), int_rect, border_radii_data };
    auto maybe_backdrop_bitmap = context.painter().get_region_bitmap(int_rect, Gfx::BitmapFormat::BGRA8888);
    if (maybe_backdrop_bitmap.is_error())
        return;
    auto backdrop_bitmap = maybe_backdrop_bitmap.release_value();
    for (auto& filter : backdrop_filter.filters()) {
        filter.function.visit([&](CSS::FilterFunction::Blur const& blur) {
      auto sigma = 0;
      if (blur.radius.has_value())
        sigma = blur.radius->resolved(node).to_px(node);
      Gfx::StackBlurFilter filter { backdrop_bitmap };
      filter.process_rgba(sigma * 2, Color::Transparent); },
            [&](auto&) {
                TODO();
            });
    }
    context.painter().blit(int_rect.location(), *backdrop_bitmap, backdrop_bitmap->rect());
}

}
