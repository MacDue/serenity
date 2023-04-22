/*
 * Copyright (c) 2020, Matthew Olsson <mattco@serenityos.org>
 * Copyright (c) 2022, Sam Atkins <atkinssj@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/GenericLexer.h>
#include <AK/Variant.h>
#include <AK/Vector.h>
#include <LibGfx/Point.h>

namespace Web::SVG {

enum class PathInstructionType {
    Move,
    ClosePath,
    Line,
    HorizontalLine,
    VerticalLine,
    Curve,
    SmoothCurve,
    QuadraticBezierCurve,
    SmoothQuadraticBezierCurve,
    EllipticalArc,
    Invalid,
};

struct PathInstruction {
    PathInstructionType type;
    bool absolute;
    Vector<float> data;
};

struct Transform {
    struct Translate {
        float x;
        float y;
    };
    struct Scale {
        float x;
        float y;
    };
    struct Rotate {
        float a;
        float x;
        float y;
    };
    struct SkewX {
        float a;
    };
    struct SkewY {
        float a;
    };
    struct Matrix {
        float a;
        float b;
        float c;
        float d;
        float e;
        float f;
    };

    using Operation = Variant<Translate, Scale, Rotate, SkewX, SkewY, Matrix>;
    Operation operation;
};

struct PreserveAspectRatio {
    enum class Align {
        None,
        xMinYMin,
        xMidYMin,
        xMaxYMin,
        xMinYMid,
        xMidYMid,
        xMaxYMid,
        xMinYMax,
        xMidYMax,
        xMaxYMax
    };
    enum class MeetOrSlice {
        Meet,
        Slice
    };
    Align align { Align::xMidYMid };
    MeetOrSlice meet_or_slice { MeetOrSlice::Meet };
};

enum class GradientUnits {
    ObjectBoundingBox,
    UserSpaceOnUse
};
class NumberPercentage {
public:
    NumberPercentage(float value, bool is_percentage)
        : m_value(is_percentage ? value / 100 : value)
        , m_is_percentage(is_percentage)
    {
    }

    static NumberPercentage create_percentage(float value)
    {
        return NumberPercentage(value, true);
    }

    static NumberPercentage create_number(float value)
    {
        return NumberPercentage(value, false);
    }

    float resolve_relative_to(float length);

    float value() { return m_value; }

private:
    float m_value;
    bool m_is_percentage { false };
};

class AttributeParser final {
public:
    ~AttributeParser() = default;

    static Optional<float> parse_coordinate(StringView input);
    static Optional<float> parse_length(StringView input);
    static Optional<NumberPercentage> parse_number_percentage(StringView input);
    static Optional<float> parse_positive_length(StringView input);
    static Vector<Gfx::FloatPoint> parse_points(StringView input);
    static Vector<PathInstruction> parse_path_data(StringView input);
    static Optional<Vector<Transform>> parse_transform(StringView input);
    static Optional<PreserveAspectRatio> parse_preserve_aspect_ratio(StringView input);
    static Optional<GradientUnits> parse_gradient_units(StringView input);

private:
    AttributeParser(StringView source);

    void parse_drawto();
    void parse_moveto();
    void parse_closepath();
    void parse_lineto();
    void parse_horizontal_lineto();
    void parse_vertical_lineto();
    void parse_curveto();
    void parse_smooth_curveto();
    void parse_quadratic_bezier_curveto();
    void parse_smooth_quadratic_bezier_curveto();
    void parse_elliptical_arc();

    Optional<Vector<Transform>> parse_transform();

    float parse_length();
    float parse_coordinate();
    Vector<float> parse_coordinate_pair();
    Vector<float> parse_coordinate_sequence();
    Vector<Vector<float>> parse_coordinate_pair_sequence();
    Vector<float> parse_coordinate_pair_double();
    Vector<float> parse_coordinate_pair_triplet();
    Vector<float> parse_elliptical_arg_argument();
    void parse_whitespace(bool must_match_once = false);
    void parse_comma_whitespace();
    float parse_number();
    float parse_nonnegative_number();
    float parse_flag();
    // -1 if negative, +1 otherwise
    int parse_sign();

    bool match_whitespace() const;
    bool match_comma_whitespace() const;
    bool match_coordinate() const;
    bool match_length() const;
    bool match_number() const;
    bool match(char c) const { return !done() && ch() == c; }

    bool done() const { return m_lexer.is_eof(); }
    char ch() const { return m_lexer.peek(); }
    char consume() { return m_lexer.consume(); }

    GenericLexer m_lexer;
    Vector<PathInstruction> m_instructions;
};

}
