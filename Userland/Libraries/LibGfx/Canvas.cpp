/*
 * Copyright (c) 2024, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Canvas.h>
#include <LibGfx/Painter.h>

namespace Gfx {

static Gfx::Path rect_path(FloatRect const& rect)
{
    Gfx::Path path;
    path.move_to({ rect.x(), rect.y() });
    path.line_to({ rect.x() + rect.width(), rect.y() });
    path.line_to({ rect.x() + rect.width(), rect.y() + rect.height() });
    path.line_to({ rect.x(), rect.y() + rect.height() });
    path.close();
    return path;
}

void Canvas::draw_glyphs(ReadonlySpan<DrawGlyphOrEmoji> run, Color color, Gfx::FloatPoint translation, float scale)
{
    auto& painter = layer().painter;
    for (auto& glyph_or_emoji : run) {
        auto transformed_glyph = glyph_or_emoji;
        transformed_glyph.visit([&](auto& glyph) {
            glyph.position = glyph.position.scaled(scale).translated(translation.translated(layer().transform.translation()));
            glyph.font = *glyph.font->with_size(glyph.font->point_size() * scale);
        });
        if (glyph_or_emoji.has<Gfx::DrawGlyph>()) {
            auto& glyph = transformed_glyph.get<Gfx::DrawGlyph>();
            painter.draw_glyph(glyph.position, glyph.code_point, *glyph.font, color);
        } else {
            auto& emoji = transformed_glyph.get<Gfx::DrawEmoji>();
            painter.draw_emoji(emoji.position.to_type<int>(), *emoji.emoji, *emoji.font);
        }
    }
}

void Canvas::fill_rect(FloatRect const& rect, Color color)
{
    if (layer().transform.is_identity_or_translation()) {
        painter().fill_rect(
            rect.translated(layer().transform.translation()), color);
        return;
    }
    auto path = rect_path(rect).copy_transformed(layer().transform);
    painter().fill_path(path, color, Painter::WindingRule::EvenOdd);
}

void Canvas::fill_rect(FloatRect const& rect, PaintStyle const& style, float opacity)
{
    auto path = rect_path(rect).copy_transformed(layer().transform);
    painter().fill_path(path, style, opacity, Painter::WindingRule::EvenOdd);
}

void Canvas::draw_line(FloatLine const& line, Color color, float thickness)
{
    Gfx::Path path;
    path.move_to(line.a());
    path.line_to(line.b());
    return fill_path(path.stroke_to_fill(thickness), color, Painter::WindingRule::EvenOdd);
}

void Canvas::draw_rect(FloatRect const& rect, Color color, float thickness)
{
    return fill_path(rect_path(rect).stroke_to_fill(thickness), color, Painter::WindingRule::EvenOdd);
}

void Canvas::fill_path(Path const& path, Color color, Painter::WindingRule winding_rule)
{
    return painter().fill_path(path.copy_transformed(layer().transform), color, winding_rule);
}

void Canvas::fill_path(Path const& path, PaintStyle const& style, float opacity, Painter::WindingRule winding_rule)
{
    return painter().fill_path(path.copy_transformed(layer().transform), style, opacity, winding_rule);
}

void Canvas::stroke_path(Path const& path, Color color, float thickness)
{
    if (thickness <= 0)
        return;
    return fill_path(path.stroke_to_fill(thickness), color);
}

void Canvas::stroke_path(Path const& path, PaintStyle const& style, float thickness, float opacity)
{
    if (thickness <= 0)
        return;
    return fill_path(path.stroke_to_fill(thickness), style, opacity);
}

void Canvas::draw_bitmap(FloatRect const& dst_rect, Bitmap const& bitmap, FloatRect const& src_rect, float opacity, Painter::ScalingMode scaling_mode)
{
    return layer().painter.draw_scaled_bitmap_with_transform(dst_rect.to_type<int>(), bitmap, src_rect, layer().transform, opacity, scaling_mode);
}

NonnullRefPtr<Bitmap> Canvas::new_bitmap() const
{
    return Gfx::Bitmap::create(BitmapFormat::BGRA8888, m_layer_stack.first().target->size()).release_value_but_fixme_should_propagate_errors();
}

void Canvas::set_clip(FloatRect const& rect)
{
    if (layer().transform.is_identity_or_translation()) {
        // FIXME: Don't clip to int!
        layer().painter.clear_clip_rect();
        layer().painter.add_clip_rect(layer().transform.map(rect).to_type<int>());
        return;
    }
    set_clip(rect_path(rect));
}

void Canvas::apply_current_clip()
{
    if (!layer().clip.mask)
        return;
    layer().target->apply_mask(*layer().clip.mask, Bitmap::MaskKind::Alpha);
    layer().clip.mask->fill(Gfx::Color::Transparent);
}

void Canvas::set_clip(Path const& path)
{
    // if (layer().clip.mask) {
    //   apply_current_clip();
    // } else {
    //   layer().clip.mask = new_bitmap();
    // }
    layer().painter.clear_clip_rect();
    // Painter painter { *layer().clip.mask };
    auto clip_path = path.copy_transformed(layer().transform);
    // AntiAliasingPainter {painter}.fill_path(clip_path, Color::Black);
    layer().painter.add_clip_rect(enclosing_int_rect(clip_path.bounding_box()));
}

void Canvas::push_layer(float opacity)
{
    if (opacity == 1.0f) {
        m_layer_stack.append(Layer {
            .clip = {},
            .opacity = 1.0f,
            .transform = layer().transform,
            .target = nullptr,
            .painter = layer().painter });
    } else {
        auto new_layer_bitmap = new_bitmap();
        m_layer_stack.append(Layer {
            .clip = {},
            .opacity = opacity,
            .transform = layer().transform,
            .target = new_layer_bitmap,
            .painter = Painter(*new_layer_bitmap) });
    }
}

void Canvas::pop_layer()
{
    VERIFY(m_layer_stack.size() > 1);
    // apply_current_clip();
    auto top_layer = m_layer_stack.take_last();
    if (top_layer.target)
        layer().painter.blit(top_layer.painter.clip_rect().top_left(),
            *top_layer.target, top_layer.painter.clip_rect(), top_layer.opacity);
}

void Canvas::flatten()
{
    while (m_layer_stack.size() > 1)
        pop_layer();
}

Canvas::Canvas(Bitmap& target)
{
    VERIFY(target.format() == BitmapFormat::BGRA8888 || target.format() == BitmapFormat::BGRx8888);
    m_layer_stack.append(Layer {
        .clip = {},
        .opacity = 1.0f,
        .transform = {},
        .target = target,
        .painter = Painter(target) });
}

}
