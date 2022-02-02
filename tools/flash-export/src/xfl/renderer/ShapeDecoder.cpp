#include "ShapeDecoder.hpp"

#include <ek/log.h>
#include <ek/assert.h>

namespace ek::xfl {

using Op = RenderCommand::Operation;

enum EdgeSelectionBit {
    EdgeSelectionBit_FillStyle0 = 1,
    EdgeSelectionBit_FillStyle1 = 2,
    EdgeSelectionBit_Stroke = 4
};


RenderCommand ShapeEdge::to_command() const {
    if (is_quadratic) {
        return {Op::curve_to, c, p1};
    } else {
        return {Op::line_to, p1};
    }
}

ShapeEdge ShapeEdge::curve(int style, vec2_t p0, vec2_t c, vec2_t p1) {
    ShapeEdge result;
    result.fill_style_idx = style;
    result.p0 = p0;
    result.c = c;
    result.p1 = p1;
    result.is_quadratic = true;
    return result;
}

ShapeEdge ShapeEdge::line(int style, vec2_t p0, vec2_t p1) {
    ShapeEdge result;
    result.fill_style_idx = style;
    result.p0 = p0;
    result.p1 = p1;
    result.is_quadratic = false;
    return result;
}

ShapeDecoder::ShapeDecoder(const TransformModel& transform) :
        transform_{transform} {

}

void ShapeDecoder::decode(const Element& el) {
    ++total_;

    const auto& matrix = transform_.matrix;

    read_fill_styles(el);
    read_line_styles(el);

    vec2_t pen = {{0.0f, 0.0f}};

    int current_fill_0;
    int current_fill_1;
    int current_line = -1;

    Array<RenderCommand> edges{};
    Array<ShapeEdge> fills{};
    Array<ShapeEdge> back_fills{};

    for (const auto& edge: el.edges) {
        bool line_started = false;
        const auto& edgeCommands = edge.commands;
        const auto& values = edge.values;
        if (edgeCommands.empty()) {
            continue;
        }

        back_fills.clear();

        bool is_new_line_style = edge.stroke_style != current_line;
        current_fill_0 = edge.fill_style_0;
        current_fill_1 = edge.fill_style_1;

        float radius = 0.0f;

        if (is_new_line_style) {
            int line_style_idx = edge.stroke_style;
            edges.push_back(line_styles_[line_style_idx]);
            current_line = line_style_idx;

            radius = line_style_idx < 1 ? 0.0f : (el.strokes[line_style_idx - 1].weight / 2.0f);
        }

        int valueIndex = 0;

        for (char cmd: edgeCommands) {

            if (cmd == '!') {
                const auto v1 = (float) values[valueIndex++];
                const auto v2 = (float) values[valueIndex++];
                const auto p = vec2_transform({{v1, v2}}, matrix);

                //if (px != penX || py != penY) {
                if (current_line > 0 && !(line_started && almost_eq_vec2(pen, p, MATH_F32_EPSILON))) {
                    extend(p, radius);
                    edges.emplace_back(Op::move_to, p);
                    line_started = true;
                }
                //}

                pen = p;
            } else if (cmd == '|' || cmd == '/') {
                const auto v1 = (float) values[valueIndex++];
                const auto v2 = (float) values[valueIndex++];
                const auto p = vec2_transform({{v1, v2}}, matrix);

                extend(p, radius);

                if (current_line > 0) {
                    edges.emplace_back(Op::line_to, p);
                } else {
                    edges.emplace_back(Op::move_to, p);
                }

                if (current_fill_0 > 0) {
                    fills.push_back(ShapeEdge::line(current_fill_0, pen, p));
                }

                if (current_fill_1 > 0) {
                    fills.push_back(ShapeEdge::line(current_fill_1, p, pen));
                }

                pen = p;
            } else if (cmd == '[' || cmd == ']') {
                const auto v1 = (float) values[valueIndex++];
                const auto v2 = (float) values[valueIndex++];
                const auto v3 = (float) values[valueIndex++];
                const auto v4 = (float) values[valueIndex++];
                const auto c = vec2_transform({{v1, v2}}, matrix);
                const auto p = vec2_transform({{v3, v4}}, matrix);
                extend(c, radius);
                extend(p, radius);

                if (current_line > 0) {
                    edges.emplace_back(Op::curve_to, c, p);
                }

                if (current_fill_0 > 0) {
                    fills.push_back(ShapeEdge::curve(current_fill_0, pen, c, p));
                }

                if (current_fill_1 > 0) {
                    fills.push_back(ShapeEdge::curve(current_fill_1, p, c, pen));
                }

                pen = p;
            } else if (cmd == 'S') {
                const auto mask = static_cast<uint32_t>(values[valueIndex++]);
                // fillStyle0
                if ((mask & EdgeSelectionBit_FillStyle0) != 0) {
                    // todo:
                }
                // fillStyle1
                if ((mask & EdgeSelectionBit_FillStyle1) != 0) {
                    // todo:
                }
                // stroke
                if ((mask & EdgeSelectionBit_Stroke) != 0) {
                    // todo:
                }
            }
        }
        for (auto& fill: back_fills) {
            fills.push_back(fill);
        }
    }
    flush_commands(edges, fills);
}


void ShapeDecoder::read_fill_styles(const Element& el) {
    auto& result = fill_styles_;
    result.clear();

    // Special null fill-style
    result.emplace_back(Op::fill_end);

    for (const auto& fill: el.fills) {
        result.emplace_back(Op::fill_begin, &fill);
    }
}

void ShapeDecoder::read_line_styles(const Element& el) {
    auto& result = line_styles_;
    result.clear();

    // Special null line-style
    result.emplace_back(Op::line_style_reset);

    for (const auto& stroke: el.strokes) {
        if (stroke.is_solid) {
            result.emplace_back(Op::line_style_setup, &stroke);
        } else {
            /// TODO: check if not solid stroke
        }
    }
}

void ShapeDecoder::flush_commands(const Array<RenderCommand>& edges, Array<ShapeEdge>& fills) {
    auto left = static_cast<int>(fills.size());
//        bool init = false;
    int current_fill = 0;
    while (left > 0) {
        auto first = fills[0];
        auto found_fill = false;
        if (current_fill > 0) {
            for (int i = 0; i < left; ++i) {
                if (fills[i].fill_style_idx == current_fill) {
                    first = fills[i];
                    fills.erase_at(i);
                    --left;
                    found_fill = true;
                    break;
                }
            }
        }
        if (!found_fill) {
            fills[0] = fills[--left];
        }
        if (first.fill_style_idx >= static_cast<int>(fill_styles_.size())) {
            log_warn("Fill Style %d not found", first.fill_style_idx);
            continue;
        }

//          if (!init) {
//              init = true;
        if (current_fill != first.fill_style_idx) {
            commands_.push_back(fill_styles_[first.fill_style_idx]);
            current_fill = first.fill_style_idx;
        }
//          }
        const vec2_t m = first.p0;

        commands_.emplace_back(Op::move_to, m);
        commands_.push_back(first.to_command());

        auto prev = first;
        bool loop = false;

        while (!loop) {
            bool found = false;
            for (int i = 0; i < left; ++i) {
                if (prev.connects(fills[i])) {
                    prev = fills[i];
                    fills[i] = fills[--left];
                    commands_.push_back(prev.to_command());
                    found = true;
                    if (prev.connects(first)) {
                        loop = true;
                    }
                    break;
                }
            }

            if (!found) {
                /*trace("Remaining:");
                for (f in 0...left)
                    fills[f].dump ();

                throw("Dangling fill : " + prev.x1 + "," + prev.y1 + "  " + prev.fillStyle);*/
                break;
            }
        }
    }

    if (!fills.empty()) {
        commands_.emplace_back(Op::fill_end);
    }

    if (!edges.empty()) {
        //trace("EDGES: " + edges.toString());
        for (const auto& e: edges) {
            commands_.push_back(e);
        }
        commands_.emplace_back(Op::line_style_reset);
    }
}

void ShapeDecoder::extend(vec2_t p, float r) {
    bounds_builder_ = aabb2_add_circle(bounds_builder_, vec3_v(p, r));
}

bool ShapeDecoder::empty() const {
    return aabb2_is_empty(bounds_builder_) || total_ == 0;
}

RenderCommandsBatch ShapeDecoder::result() const {
    RenderCommandsBatch res;
    res.transform = transform_;
    res.bounds = bounds_builder_;
    res.total = total_;
    res.commands = commands_;
    return res;
}

}