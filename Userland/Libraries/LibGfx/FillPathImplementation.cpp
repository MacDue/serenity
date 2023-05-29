/*
 * Copyright (c) 2021, Ali Mohammad Pur <mpfard@serenityos.org>
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Debug.h>
#include <AK/QuickSort.h>
#include <LibGfx/Color.h>
#include <LibGfx/Painter.h>
#include <LibGfx/Path.h>
// #include <LibGfx/Font/PathRasterizer.h>
#include <LibGfx/EdgeFlagPathRasterizer.h>

#if defined(AK_COMPILER_GCC)
#    pragma GCC optimize("O3")
#endif

namespace Gfx {

template<Painter::FillPathMode fill_path_mode, typename ColorOrFunction>
void Painter::fill_path_impl(Path const& path, ColorOrFunction color, Gfx::Painter::WindingRule winding_rule, Optional<FloatPoint> offset)
{
    if (winding_rule != Gfx::Painter::WindingRule::EvenOdd)
        return;
    (void)color;
    (void)winding_rule;
    (void)offset;
    auto foo = path.copy_transformed(AffineTransform {}.translate(offset.value_or({})));
    auto bounding_box = enclosing_int_rect(foo.bounding_box());
    if (bounding_box.size().is_empty())
        return;
    EdgeFlagPathRasterizer<32> rasterizer(bounding_box.size());
    auto temp = foo.copy_transformed(AffineTransform {}.translate(-bounding_box.top_left().to_type<float>()));
    rasterizer.draw_path(temp);
    auto result = rasterizer.accumulate();

    constexpr bool has_constant_color = IsSameIgnoringCV<ColorOrFunction, Color>;
    auto get_color = [&](int) {
        if constexpr (has_constant_color) {
            return color;
        } else {
            return color({ 0, 0 });
        }
    };

    blit_filtered(bounding_box.top_left(), *result, result->rect(), [&](Gfx::Color color) -> Color {
        // if (color.alpha() == 100) {
        //     return Color::Red;
        // } else if (color.alpha() == 50) {
        //     return Color::Blue;
        // }
        // return Color();
        auto foo = get_color(0);
        return get_color(0).with_alpha((foo.alpha() * color.alpha()) / 255);
    });
}

void Painter::fill_path(Path const& path, Color color, WindingRule winding_rule)
{
    VERIFY(scale() == 1); // FIXME: Add scaling support.
    fill_path_impl<FillPathMode::PlaceOnIntGrid>(path, color, winding_rule);
}

void Painter::fill_path(Path const& path, PaintStyle const& paint_style, Painter::WindingRule rule)
{
    VERIFY(scale() == 1); // FIXME: Add scaling support.
    paint_style.paint(enclosing_int_rect(path.bounding_box()), [&](PaintStyle::SamplerFunction sampler) {
        fill_path_impl<FillPathMode::PlaceOnIntGrid>(path, move(sampler), rule);
    });
}

void Painter::antialiased_fill_path(Path const& path, Color color, WindingRule rule, FloatPoint translation)
{
    VERIFY(scale() == 1); // FIXME: Add scaling support.
    fill_path_impl<FillPathMode::AllowFloatingPoints>(path, color, rule, translation);
}

void Painter::antialiased_fill_path(Path const& path, PaintStyle const& paint_style, WindingRule rule, FloatPoint translation)
{
    VERIFY(scale() == 1); // FIXME: Add scaling support.
    paint_style.paint(enclosing_int_rect(path.bounding_box()), [&](PaintStyle::SamplerFunction sampler) {
        fill_path_impl<FillPathMode::AllowFloatingPoints>(path, move(sampler), rule, translation);
    });
}

}
