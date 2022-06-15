#pragma once

#include <LibGfx/AntiAliasingPainter.h>
#include <LibWeb/Painting/BorderPainting.h>

namespace Web::Painting {

class BorderRadiusCornerClipper {
public:
    static ErrorOr<BorderRadiusCornerClipper> try_create(Gfx::IntRect const& border_rect, BorderRadiiData const& border_radii);

    void sample_under_corners(Gfx::Painter& page_painter);
    void blit_corner_clipping(Gfx::Painter& page_painter);

private:
    using CornerRadius = Gfx::AntiAliasingPainter::CornerRadius;
    struct CornerData {
        struct CornerRadii {
            CornerRadius top_left;
            CornerRadius top_right;
            CornerRadius bottom_right;
            CornerRadius bottom_left;
        } corner_radii;
        struct CornerLocations {
            Gfx::IntPoint top_left;
            Gfx::IntPoint top_right;
            Gfx::IntPoint bottom_right;
            Gfx::IntPoint bottom_left;
        };
        CornerLocations page_locations;
        CornerLocations bitmap_locations;
        Gfx::IntSize corner_bitmap_size;
    } m_data;

    NonnullRefPtr<Gfx::Bitmap> m_corner_bitmap;
    bool m_has_sampled { false };

    BorderRadiusCornerClipper(CornerData corner_data, NonnullRefPtr<Gfx::Bitmap> corner_bitmap)
        : m_data { corner_data }
        , m_corner_bitmap { corner_bitmap }
    {
    }
};

}
