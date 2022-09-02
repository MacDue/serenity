/*
 * Copyright (c) 2022, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Filters/BrightnessFilter.h>
#include <LibGfx/Filters/ContrastFilter.h>
#include <LibGfx/Filters/GrayscaleFilter.h>
#include <LibGfx/Filters/InvertFilter.h>
#include <LibGfx/Filters/OpacityFilter.h>
#include <LibGfx/Filters/SepiaFilter.h>
#include <LibGfx/Filters/StackBlurFilter.h>
#include <LibWeb/Layout/Node.h>
#include <LibWeb/Painting/BorderRadiusCornerClipper.h>
#include <LibWeb/Painting/FilterPainting.h>

namespace Web::Painting {

void apply_filter_list(Gfx::Bitmap& target_bitmap, Layout::Node const& node, Span<CSS::FilterFunction const> filter_list)
{
    for (auto& filter : filter_list) {
        filter.function.visit(
            [&](CSS::FilterFunction::Blur const& blur) {
                auto sigma = 0;
                if (blur.radius.has_value())
                    sigma = blur.radius->resolved(node).to_px(node);
                Gfx::StackBlurFilter filter { target_bitmap };
                filter.process_rgba(sigma * 2, Color::Transparent);
            },
            [&](CSS::FilterFunction::Color const& color) {
                auto amount = 1.0f;
                if (color.amount.has_value()) {
                    if (color.amount->is_percentage())
                        amount = color.amount->percentage().as_fraction();
                    else
                        amount = color.amount->number().value();
                }
                auto amount_clammped = clamp(amount, 0.0f, 1.0f);

                auto apply_color_filter = [&](Gfx::ColorFilter const& filter) {
                    const_cast<Gfx::ColorFilter&>(filter).apply(target_bitmap, target_bitmap.rect(), target_bitmap, target_bitmap.rect());
                };

                switch (color.operation) {
                case CSS::FilterFunction::Color::Operation::Grayscale: {
                    apply_color_filter(Gfx::GrayscaleFilter { amount_clammped });
                    break;
                }
                case CSS::FilterFunction::Color::Operation::Brightness: {
                    apply_color_filter(Gfx::BrightnessFilter { amount });
                    break;
                }
                case CSS::FilterFunction::Color::Operation::Contrast: {
                    apply_color_filter(Gfx::ContrastFilter { amount_clammped });
                    break;
                }
                case CSS::FilterFunction::Color::Operation::Invert: {
                    apply_color_filter(Gfx::InvertFilter { amount_clammped });
                    break;
                }
                case CSS::FilterFunction::Color::Operation::Opacity: {
                    apply_color_filter(Gfx::OpacityFilter { amount_clammped });
                    break;
                }
                case CSS::FilterFunction::Color::Operation::Sepia: {
                    apply_color_filter(Gfx::SepiaFilter { amount_clammped });
                    break;
                }
                case CSS::FilterFunction::Color::Operation::Saturate: {
                    dbgln("TODO: Implement saturate() filter function!");
                    break;
                }
                default:
                    break;
                }
            },
            [&](CSS::FilterFunction::HueRotate const&) {
                dbgln("TODO: Implement hue-rotate() filter function!");
            },
            [&](CSS::FilterFunction::DropShadow const&) {
                dbgln("TODO: Implement drop-shadow() filter function!");
            });
    }
}

void apply_backdrop_filter(PaintContext& context, Layout::Node const& node, Gfx::FloatRect const& backdrop_rect, BorderRadiiData const& border_radii_data, CSS::BackdropFilter const& backdrop_filter)
{
    auto backdrop_region = backdrop_rect.to_rounded<int>();
    ScopedCornerRadiusClip corner_clipper { context.painter(), backdrop_region, border_radii_data };
    auto maybe_backdrop_bitmap = context.painter().get_region_bitmap(backdrop_region, Gfx::BitmapFormat::BGRA8888);
    if (maybe_backdrop_bitmap.is_error()) {
        dbgln("Failed get region bitmap for backdrop-filter");
        return;
    }
    auto backdrop_bitmap = maybe_backdrop_bitmap.release_value();
    apply_filter_list(*backdrop_bitmap, node, backdrop_filter.filters());
    context.painter().blit(backdrop_region.location(), *backdrop_bitmap, backdrop_bitmap->rect());
}

}
