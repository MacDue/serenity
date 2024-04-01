/*
 * Copyright (c) 2023, Aliaksandr Kalenik <kalenik.aliaksandr@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibGfx/Filters/StackBlurFilter.h>
#include <LibGfx/StylePainter.h>
#include <LibWeb/CSS/ComputedValues.h>
#include <LibWeb/Painting/BorderRadiusCornerClipper.h>
#include <LibWeb/Painting/CommandExecutorCPU.h>
#include <LibWeb/Painting/FilterPainting.h>
#include <LibWeb/Painting/RecordingPainter.h>
#include <LibWeb/Painting/ShadowPainting.h>

namespace Web::Painting {

CommandExecutorCPU::CommandExecutorCPU(Gfx::Bitmap& bitmap)
    : m_target_bitmap(bitmap)
    , m_canvas(bitmap)
{
}

CommandResult CommandExecutorCPU::draw_glyph_run(Vector<Gfx::DrawGlyphOrEmoji> const& run, Color const& color, Gfx::FloatPoint t, double s)
{
    m_canvas.draw_glyphs(run, color, t, (float)s);
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::draw_text(Gfx::IntRect const&, String const&, Gfx::TextAlignment, Color const&, Gfx::TextElision, Gfx::TextWrapping, Optional<NonnullRefPtr<Gfx::Font>> const&)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::fill_rect(Gfx::IntRect const& rect, Color const& color, Vector<Gfx::Path> const&)
{
    m_canvas.fill_rect(rect.to_type<float>(), color);
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::draw_scaled_bitmap(Gfx::IntRect const& dst_rect, Gfx::Bitmap const& bitmap, Gfx::IntRect const& src_rect, Gfx::Painter::ScalingMode scaling_mode)
{
    m_canvas.draw_bitmap(dst_rect.to_type<float>(), bitmap, src_rect.to_type<float>(), 1.0, scaling_mode);
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::draw_scaled_immutable_bitmap(Gfx::IntRect const& dst_rect, Gfx::ImmutableBitmap const& immutable_bitmap, Gfx::IntRect const& src_rect, Gfx::Painter::ScalingMode scaling_mode, Vector<Gfx::Path> const&)
{
    m_canvas.draw_bitmap(dst_rect.to_type<float>(), immutable_bitmap.bitmap(), src_rect.to_type<float>(), 1.0, scaling_mode);
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::set_clip_rect(Gfx::IntRect const& rect)
{
    m_canvas.set_clip(rect.to_type<float>());
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::clear_clip_rect()
{
    m_canvas.clear_clip();
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::push_stacking_context(
    float opacity, bool, Gfx::IntRect const& source_paintable_rect, Gfx::IntPoint post_transform_translation,
    CSS::ImageRendering, StackingContextTransform transform, Optional<StackingContextMask>)
{
    // FIXME: is_fixed_position and masks!
    auto affine_transform = Gfx::extract_2d_affine_transform(transform.matrix);
    auto paint_transform = Gfx::AffineTransform {}
                               .set_translation(post_transform_translation.to_type<float>())
                               .translate(transform.origin)
                               .multiply(affine_transform)
                               .translate(-transform.origin);
    m_canvas.push_layer(opacity);
    m_canvas.apply_transform(paint_transform);
    m_canvas.set_clip(source_paintable_rect.to_type<float>());
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::pop_stacking_context()
{
    m_canvas.pop_layer();
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::paint_linear_gradient(Gfx::IntRect const&, Web::Painting::LinearGradientData const&, Vector<Gfx::Path> const&)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::paint_outer_box_shadow(PaintOuterBoxShadowParams const&)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::paint_inner_box_shadow(PaintOuterBoxShadowParams const&)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::paint_text_shadow(int, Gfx::IntRect const&, Gfx::IntRect const&, Span<Gfx::DrawGlyphOrEmoji const>, Color const&, int, Gfx::IntPoint const&)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::fill_rect_with_rounded_corners(Gfx::IntRect const&, Color const&, Gfx::AntiAliasingPainter::CornerRadius const&, Gfx::AntiAliasingPainter::CornerRadius const&, Gfx::AntiAliasingPainter::CornerRadius const&, Gfx::AntiAliasingPainter::CornerRadius const&, Vector<Gfx::Path> const&)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::fill_path_using_color(Gfx::Path const& path, Color const& color, Gfx::Painter::WindingRule winding_rule, Gfx::FloatPoint const& aa_translation)
{
    m_canvas.fill_path(path.copy_transformed(Gfx::AffineTransform {}.set_translation(aa_translation)), color, winding_rule);
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::fill_path_using_paint_style(Gfx::Path const& path, Gfx::PaintStyle const& paint_style, Gfx::Painter::WindingRule winding_rule, float opacity, Gfx::FloatPoint const& aa_translation)
{
    m_canvas.fill_path(path.copy_transformed(Gfx::AffineTransform {}.set_translation(aa_translation)), paint_style, opacity, winding_rule);
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::stroke_path_using_color(Gfx::Path const& path, Color const& color, float thickness, Gfx::FloatPoint const& aa_translation)
{
    m_canvas.stroke_path(path.copy_transformed(Gfx::AffineTransform {}.set_translation(aa_translation)), color, thickness);
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::stroke_path_using_paint_style(Gfx::Path const& path, Gfx::PaintStyle const& paint_style, float thickness, float opacity, Gfx::FloatPoint const& aa_translation)
{
    m_canvas.stroke_path(path.copy_transformed(Gfx::AffineTransform {}.set_translation(aa_translation)), paint_style, thickness, opacity);
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::draw_ellipse(Gfx::IntRect const&, Color const&, int)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::fill_ellipse(Gfx::IntRect const&, Color const&, Gfx::AntiAliasingPainter::BlendMode)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::draw_line(Color const& color, Gfx::IntPoint const& a, Gfx::IntPoint const& b, int thickness, Gfx::Painter::LineStyle, Color const&)
{
    m_canvas.draw_line(Gfx::FloatLine(a, b), color, thickness);
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::draw_signed_distance_field(Gfx::IntRect const&, Color const&, Gfx::GrayscaleBitmap const&, float)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::paint_frame(Gfx::IntRect const&, Palette const&, Gfx::FrameStyle)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::apply_backdrop_filter(Gfx::IntRect const&, Web::CSS::ResolvedBackdropFilter const&)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::draw_rect(Gfx::IntRect const& rect, Color const& color, bool)
{
    m_canvas.draw_rect(rect.to_type<float>(), color, 1);
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::paint_radial_gradient(Gfx::IntRect const&, Web::Painting::RadialGradientData const&, Gfx::IntPoint const&, Gfx::IntSize const&, Vector<Gfx::Path> const&)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::paint_conic_gradient(Gfx::IntRect const&, Web::Painting::ConicGradientData const&, Gfx::IntPoint const&, Vector<Gfx::Path> const&)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::draw_triangle_wave(Gfx::IntPoint const&, Gfx::IntPoint const&, Color const&, int, int)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::sample_under_corners(u32, CornerRadii const&, Gfx::IntRect const&, CornerClip)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::blit_corner_clipping(u32)
{
    return CommandResult::Continue;
}

CommandResult CommandExecutorCPU::paint_borders(DevicePixelRect const& border_rect, CornerRadii const& corner_radii, BordersDataDevicePixels const& borders_data)
{
    paint_all_borders(m_canvas, border_rect, corner_radii, borders_data);
    return CommandResult::Continue;
}

bool CommandExecutorCPU::would_be_fully_clipped_by_painter(Gfx::IntRect) const
{
    return false;
}

}
