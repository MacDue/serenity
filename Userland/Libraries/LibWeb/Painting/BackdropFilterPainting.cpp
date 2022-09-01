/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Filters/GrayscaleFilter.h>
#include <LibGfx/Filters/InvertFilter.h>
#include <LibGfx/Filters/SepiaFilter.h>
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
        filter.function.visit(
            [&](CSS::FilterFunction::Blur const& blur) {
                auto sigma = 0;
                if (blur.radius.has_value())
                    sigma = blur.radius->resolved(node).to_px(node);
                Gfx::StackBlurFilter filter { backdrop_bitmap };
                filter.process_rgba(sigma * 2, Color::Transparent);
            },
            [&](CSS::FilterFunction::Color const& color) {
                [[maybe_unused]] auto amount = 1;
                if (color.amount.has_value())
                    amount = color.amount->resolved(node, CSS::Number(CSS::Number::Type::Number, 1.0f)).value();
                switch (color.operation) {
                case CSS::FilterFunction::Color::Operation::Grayscale: {
                    Gfx::GrayscaleFilter filter { clamp(amount, 0, 1) };
                    filter.apply(*backdrop_bitmap, backdrop_bitmap->rect(), *backdrop_bitmap, backdrop_bitmap->rect());
                    break;
                }
                case CSS::FilterFunction::Color::Operation::Brightness: {

                    break;
                }
                case CSS::FilterFunction::Color::Operation::Contrast: {

                    break;
                }
                case CSS::FilterFunction::Color::Operation::Invert: {
                    Gfx::InvertFilter filter { clamp(amount, 0, 1) };
                    filter.apply(*backdrop_bitmap, backdrop_bitmap->rect(), *backdrop_bitmap, backdrop_bitmap->rect());
                    break;
                }
                case CSS::FilterFunction::Color::Operation::Opacity: {

                    break;
                }
                case CSS::FilterFunction::Color::Operation::Sepia: {
                    Gfx::SepiaFilter filter { clamp(amount, 0, 1) };
                    filter.apply(*backdrop_bitmap, backdrop_bitmap->rect(), *backdrop_bitmap, backdrop_bitmap->rect());
                    break;
                }
                case CSS::FilterFunction::Color::Operation::Saturate: {

                    break;
                }
                default:
                    break;
                }
            },
            [&](auto&) {
                TODO();
            });
    }
    context.painter().blit(int_rect.location(), *backdrop_bitmap, backdrop_bitmap->rect());
}

}
