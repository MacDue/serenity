#pragma once

#include <LibGfx/Color.h>
#include <LibGfx/Bitmap.h>
#include <LibGfx/AntiAliasingPainter.h>

namespace Gfx {

struct ShadowPainter {
  using CornerRadius = AntiAliasingPainter::CornerRadius;

  ShadowPainter(Color color, int blur_radius, int spread_distance)
    : m_color{color}, m_blur_radius{blur_radius}, m_spread_distance { spread_distance } {}

  enum class Infill {
    Filled,
    Clipped
  };

  void paint_shadow_around(Gfx::IntRect const & rect, CornerRadius top_left, CornerRadius top_right, CornerRadius bottom_right, CornerRadius bottom_left, Infill infill = Infill::Clipped);

  void paint_shadow_around(Gfx::IntRect const & rect, Infill infill = Infill::Clipped) {
    paint_shadow_around(rect, {}, {}, {}, {}, infill);
  }

private:
  Color m_color;
  int m_blur_radius;
  int m_spread_distance;
  RefPtr<Bitmap> m_shadow_bitmap;
};

}
