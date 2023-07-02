/*
 * Copyright (c) 2023, MacDue <macdue@dueutil.tech>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Array.h>
#include <AK/Endian.h>
#include <AK/FixedArray.h>
#include <AK/LEB128.h>
#include <AK/MemoryStream.h>
#include <AK/Variant.h>
#include <LibGfx/Color.h>
#include <LibGfx/Line.h>
#include <LibGfx/PaintStyle.h>
#include <LibGfx/Painter.h>
#include <LibGfx/Path.h>
#include <LibGfx/Point.h>
#include <LibGfx/VectorFormats/TinyVGRender.h>

namespace Gfx {

using VarUInt = LEB128<u32>;

static constexpr Array<u8, 2> TVG_MAGIC { 0x72, 0x56 };

enum class ColorEncoding : u8 {
    RGBA8888 = 0,
    RGB565 = 1,
    RGBAF32 = 2,
    Custom = 3
};

enum class CoordinateRange : u8 {
    Default = 0,
    Reduced = 1,
    Enhanced = 2
};

enum class StyleType : u8 {
    FlatColored = 0,
    LinearGradient = 1,
    RadialGradinet = 2
};

enum class Command : u8 {
    EndOfDocument = 0,
    FillPolygon = 1,
    FillRectangles = 2,
    FillPath = 3,
    DrawLines = 4,
    DrawLineLoop = 5,
    DrawLineStrip = 6,
    DrawLinePath = 7,
    OutlineFillPolygon = 8,
    OutlineFillRectangles = 9,
    OutLineFillPath = 10
};

enum class PathCommand : u8 {
    Line = 0,
    HorizontalLine = 1,
    VerticalLine = 2,
    CubicBezier = 3,
    ArcCircle = 4,
    ArcEllipse = 5,
    ClosePath = 6,
    QuadraticBezier = 7
};

using Style = Variant<Color, NonnullRefPtr<SVGGradientPaintStyle>>;

struct TinyVGHeader {
    u8 version;
    u8 scale;
    ColorEncoding color_encoding;
    CoordinateRange coordinate_range;
    u32 width;
    u32 height;
    u32 color_count;
};

static ErrorOr<TinyVGHeader> decode_tinyvg_header(Stream& stream)
{
    Array<u8, 2> magic_bytes;
    TRY(stream.read_until_filled(magic_bytes));
    if (magic_bytes != TVG_MAGIC)
        return Error::from_string_literal("Invalid TVG: incorrect header magic");
    u8 version = TRY(stream.read_value<u8>());
    u8 properties = TRY(stream.read_value<u8>());
    u8 scale = properties & 0xF;
    auto color_encoding = static_cast<ColorEncoding>((properties >> 4) & 0x3);
    auto coordinate_range = static_cast<CoordinateRange>((properties >> 6) & 0x3);
    u32 width;
    u32 height;
    switch (coordinate_range) {
    case CoordinateRange::Default:
        width = TRY(stream.read_value<u16>());
        height = TRY(stream.read_value<u16>());
        break;
    case CoordinateRange::Reduced:
        width = TRY(stream.read_value<u8>());
        height = TRY(stream.read_value<u8>());
        break;
    case CoordinateRange::Enhanced:
        width = TRY(stream.read_value<u32>());
        height = TRY(stream.read_value<u32>());
        break;
    default:
        VERIFY_NOT_REACHED();
    }
    auto color_count = TRY(stream.read_value<VarUInt>());
    return TinyVGHeader {
        .version = version,
        .scale = scale,
        .color_encoding = color_encoding,
        .coordinate_range = coordinate_range,
        .width = width,
        .height = height,
        .color_count = color_count
    };
}

static ErrorOr<FixedArray<Color>> decode_color_table(Stream& stream, ColorEncoding encoding, u32 color_count)
{
    if (encoding == ColorEncoding::Custom)
        return Error::from_string_literal("Unsupported TVG color encoding");
    auto color_table = TRY(FixedArray<Color>::create(color_count));
    auto parse_color = [&]() -> ErrorOr<Color> {
        switch (encoding) {
        case ColorEncoding::RGBA8888: {
            Array<u8, 4> rgba;
            TRY(stream.read_until_filled(rgba));
            return Color(rgba[0], rgba[1], rgba[2], rgba[3]);
        }
        case ColorEncoding::RGB565: {
            u16 color = TRY(stream.read_value<u16>());
            auto red = (color >> (6 + 5)) & 0x1f;
            auto green = (color >> 5) & 0x3f;
            auto blue = (color >> 0) & 0x1f;
            return Color((red * 255 + 15) / 31, (green * 255 + 31), (blue * 255 + 15) / 31);
        }
        case ColorEncoding::RGBAF32: {
            auto red = TRY(stream.read_value<f32>());
            auto green = TRY(stream.read_value<f32>());
            auto blue = TRY(stream.read_value<f32>());
            auto alpha = TRY(stream.read_value<f32>());
            return Color(red * 255, green * 255, blue * 255, alpha * 255);
        }
        default:
            VERIFY_NOT_REACHED();
        }
    };
    for (auto& color : color_table) {
        color = TRY(parse_color());
    }
    return color_table;
}

class TinyVGReader {
public:
    TinyVGReader(Stream& stream, TinyVGHeader const& header, ReadonlySpan<Color> color_table)
        : m_stream(stream)
        , m_scale(pow<float>(0.5, header.scale))
        , m_coordinate_range(header.coordinate_range)
        , m_color_table(color_table)
    {
    }

    ErrorOr<float> read_unit()
    {
        auto read_value = [&]() -> ErrorOr<i32> {
            switch (m_coordinate_range) {
            case CoordinateRange::Default:
                return TRY(m_stream.read_value<i16>());
            case CoordinateRange::Reduced:
                return TRY(m_stream.read_value<i8>());
            case CoordinateRange::Enhanced:
                return TRY(m_stream.read_value<i32>());
            default:
                VERIFY_NOT_REACHED();
            }
        };
        return TRY(read_value()) * m_scale;
    }

    ErrorOr<u32> read_var_uint()
    {
        return TRY(m_stream.read_value<VarUInt>());
    }

    ErrorOr<FloatPoint> read_point()
    {
        return FloatPoint { TRY(read_unit()), TRY(read_unit()) };
    }

    ErrorOr<Style> read_style(StyleType type)
    {
        auto read_color = [&]() -> ErrorOr<Color> {
            auto color_index = TRY(m_stream.read_value<VarUInt>());
            return m_color_table[color_index];
        };
        switch (type) {
        case StyleType::FlatColored: {
            return TRY(read_color());
        }
        case StyleType::LinearGradient:
        case StyleType::RadialGradinet: {
            // TODO: Make PaintStyle (for these ultra basic gradients)
            [[maybe_unused]] auto point_0 = TRY(read_point());
            [[maybe_unused]] auto point_1 = TRY(read_point());
            [[maybe_unused]] auto color_0 = TRY(read_color());
            [[maybe_unused]] auto color_1 = TRY(read_color());
            return Color(Color::Black);
        }
        }
        VERIFY_NOT_REACHED();
    }

    ErrorOr<FloatRect> read_rectangle()
    {
        return FloatRect { TRY(read_unit()), TRY(read_unit()), TRY(read_unit()), TRY(read_unit()) };
    }

    ErrorOr<FloatLine> read_line()
    {
        return FloatLine { TRY(read_point()), TRY(read_point()) };
    }

    ErrorOr<Path> read_path(u32 segment_count)
    {
        Path path;
        auto segment_lengths = TRY(FixedArray<u32>::create(segment_count));
        for (auto& command_count : segment_lengths) {
            command_count = TRY(read_var_uint()) + 1;
        }
        for (auto command_count : segment_lengths) {
            auto start_point = TRY(read_point());
            path.move_to(start_point);
            for (u32 i = 0; i < command_count; i++) {
                u8 command_tag = TRY(m_stream.read_value<u8>());
                auto path_command = static_cast<PathCommand>(command_tag & 0x7);
                switch (path_command) {
                case PathCommand::Line:
                    path.line_to(TRY(read_point()));
                    break;
                case PathCommand::HorizontalLine:
                    path.line_to({ TRY(read_unit()), path.segments().last()->point().y() });
                    break;
                case PathCommand::VerticalLine:
                    path.line_to({ path.segments().last()->point().x(), TRY(read_unit()) });
                    break;
                case PathCommand::CubicBezier: {
                    auto control_0 = TRY(read_point());
                    auto control_1 = TRY(read_point());
                    auto point_1 = TRY(read_point());
                    path.cubic_bezier_curve_to(control_0, control_1, point_1);
                    break;
                }
                case PathCommand::ArcCircle: {
                    u8 flags = TRY(m_stream.read_value<u8>());
                    bool large_arc = (flags >> 0) & 0b1;
                    bool sweep = (flags >> 1) & 0b1;
                    auto radius = TRY(read_unit());
                    auto target = TRY(read_point());
                    path.arc_to(target, radius, large_arc, sweep);
                    break;
                }
                case PathCommand::ArcEllipse: {
                    u8 flags = TRY(m_stream.read_value<u8>());
                    bool large_arc = (flags >> 0) & 0b1;
                    bool sweep = (flags >> 1) & 0b1;
                    auto radius_x = TRY(read_unit());
                    auto radius_y = TRY(read_unit());
                    auto rotation = TRY(read_unit());
                    auto target = TRY(read_point());
                    path.elliptical_arc_to(target, { radius_x, radius_y }, rotation, large_arc, sweep);
                    break;
                }
                case PathCommand::ClosePath: {
                    path.close();
                    break;
                }
                case PathCommand::QuadraticBezier: {
                    auto control = TRY(read_point());
                    auto point_1 = TRY(read_point());
                    path.quadratic_bezier_curve_to(control, point_1);
                    break;
                }
                default:
                    VERIFY_NOT_REACHED();
                }
            }
        }
        return path;
    }

private:
    Stream& m_stream;
    float m_scale {};
    CoordinateRange m_coordinate_range;
    ReadonlySpan<Color> m_color_table;
};

struct DrawCommand {
    Path path;
    Optional<Style> fill {};
    Optional<Style> stroke {};
    float stroke_width { 0.0f };
};

struct TinyVG {
    ErrorOr<RefPtr<Gfx::Bitmap>> bitmap(IntSize size);

private:
    static ErrorOr<TinyVG> decode(Stream& stream);

    TinyVG(IntSize size, Vector<DrawCommand> draw_commands)
        : m_size(size)
        , m_draw_commands(move(draw_commands))
    {
    }

    IntSize m_size;
    Vector<DrawCommand> m_draw_commands;
};

ErrorOr<TinyVG> TinyVG::decode(Stream& stream)
{
    auto header = TRY(decode_tinyvg_header(stream));
    if (header.version != 1)
        return Error::from_string_literal("Unsupported TVG version");

    auto color_table = TRY(decode_color_table(stream, header.color_encoding, header.color_count));
    TinyVGReader reader { stream, header, color_table.span() };

    auto rectangle_to_path = [](FloatRect const& rect) -> Path {
        Path path;
        path.move_to({ rect.x(), rect.y() });
        path.line_to({ rect.x() + rect.width(), rect.y() });
        path.line_to({ rect.x() + rect.width(), rect.y() + rect.height() });
        path.line_to({ rect.x(), rect.y() + rect.height() });
        path.close();
        return path;
    };

    Vector<DrawCommand> draw_commands;
    while (true) {
        u8 command_info = TRY(stream.read_value<u8>());
        auto command = static_cast<Command>(command_info & 0x3f);
        auto style_type = static_cast<StyleType>((command_info >> 6) & 0x3);

        switch (command) {
        case Command::EndOfDocument:
            break;
        case Command::FillPolygon: {
            u32 point_count = TRY(reader.read_var_uint());
            auto style = TRY(reader.read_style(style_type));
            Path polygon;
            polygon.move_to(TRY(reader.read_point()));
            for (u32 i = 0; i < point_count; i++)
                polygon.line_to(TRY(reader.read_point()));
            TRY(draw_commands.try_append(DrawCommand { move(polygon), move(style) }));
            break;
        }
        case Command::FillRectangles: {
            u32 rectangle_count = TRY(reader.read_var_uint()) + 1;
            auto style = TRY(reader.read_style(style_type));
            for (u32 i = 0; i < rectangle_count; i++) {
                TRY(draw_commands.try_append(DrawCommand {
                    rectangle_to_path(TRY(reader.read_rectangle())), move(style) }));
            }
            break;
        }
        case Command::FillPath: {
            u32 segment_count = TRY(reader.read_var_uint()) + 1;
            auto style = TRY(reader.read_style(style_type));
            auto path = TRY(reader.read_path(segment_count));
            TRY(draw_commands.try_append(DrawCommand { move(path), move(style) }));
            break;
        }
        case Command::DrawLines: {
            u32 line_count = TRY(reader.read_var_uint()) + 1;
            auto line_style = TRY(reader.read_style(style_type));
            auto line_width = TRY(reader.read_unit());
            Path path;
            for (u32 i = 0; i < line_count; i++) {
                auto line = TRY(reader.read_line());
                path.move_to(line.a());
                path.line_to(line.b());
            }
            TRY(draw_commands.try_append(DrawCommand { move(path), {}, line_style, line_width }));
            break;
        }
        case Command::DrawLineStrip:
        case Command::DrawLineLoop: {
            u32 point_count = TRY(reader.read_var_uint());
            auto line_style = TRY(reader.read_style(style_type));
            auto line_width = TRY(reader.read_unit());
            Path path;
            path.move_to(TRY(reader.read_point()));
            for (u32 i = 0; i < point_count; i++)
                path.line_to(TRY(reader.read_point()));
            if (command == Command::DrawLineLoop)
                path.close();
            TRY(draw_commands.try_append(DrawCommand { move(path), {}, line_style, line_width }));
            break;
        }
        case Command::DrawLinePath: {
            u32 segment_count = TRY(reader.read_var_uint()) + 1;
            auto line_style = TRY(reader.read_style(style_type));
            auto line_width = TRY(reader.read_unit());
            auto path = TRY(reader.read_path(segment_count));
            TRY(draw_commands.try_append(DrawCommand { move(path), {}, line_style, line_width }));
            break;
        }
        case Command::OutlineFillPolygon: {

            break;
        }
        case Command::OutlineFillRectangles: {

            break;
        }
        case Command::OutLineFillPath: {

            break;
        }
        default:
            VERIFY_NOT_REACHED();
        }
    }

    return TinyVG { { header.width, header.height }, move(draw_commands) };
}

}
