#include <ek/logger.hpp>
#include "shape_edge_decoder.h"

namespace ek::flash {

using render_op = render_command::operation;

shape_decoder::shape_decoder(const transform_model& transform)
        : transform_{transform} {

}

void shape_decoder::decode(const element_t& el) {
    ++total_;

    const auto& matrix = transform_.matrix;

    read_fill_styles(el);
    read_line_styles(el);

    float2 pen{0.0f, 0.0f};

    int current_fill_0;
    int current_fill_1;
    int current_line = -1;

    std::vector<render_command> edges;
    std::vector<shape_edge> fills;
    std::vector<shape_edge> back_fills;

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

            radius = line_style_idx < 1 ? 0.0f : (el.strokes[line_style_idx - 1].solid.weight / 2.0f);
        }

        int valueIndex = 0;

        for (char cmd : edgeCommands) {

            if (cmd == '!') {

                auto v1 = values[valueIndex++];
                auto v2 = values[valueIndex++];
                auto p = matrix.transform(v1, v2);

                //if (px != penX || py != penY) {
                if (current_line > 0 && !(line_started && equals(pen, p))) {
                    extend(p, radius);
                    edges.emplace_back(render_op::move_to, p);
                    line_started = true;
                }
                //}

                pen = p;
            } else if (cmd == '|' || cmd == '/') {
                auto v1 = values[valueIndex++];
                auto v2 = values[valueIndex++];

                auto p = matrix.transform(v1, v2);
                extend(p, radius);

                if (current_line > 0) {
                    edges.emplace_back(render_op::line_to, p);
                } else {
                    edges.emplace_back(render_op::move_to, p);
                }

                if (current_fill_0 > 0) {
                    fills.push_back(shape_edge::line(current_fill_0, pen, p));
                }

                if (current_fill_1 > 0) {
                    fills.push_back(shape_edge::line(current_fill_1, p, pen));
                }

                pen = p;
            } else if (cmd == '[' || cmd == ']') {
                auto v1 = values[valueIndex++];
                auto v2 = values[valueIndex++];
                auto v3 = values[valueIndex++];
                auto v4 = values[valueIndex++];
                auto c = matrix.transform(v1, v2);
                auto p = matrix.transform(v3, v4);
                extend(c, radius);
                extend(p, radius);

                if (current_line > 0) {
                    edges.emplace_back(render_op::curve_to, c, p);
                }

                if (current_fill_0 > 0) {
                    fills.push_back(shape_edge::curve(current_fill_0, pen, c, p));
                }

                if (current_fill_1 > 0) {
                    fills.push_back(shape_edge::curve(current_fill_1, p, c, pen));
                }

                pen = p;
            }
        }
        fills.insert(fills.end(), back_fills.begin(), back_fills.end());
    }
    flush_commands(edges, fills);
}


void shape_decoder::read_fill_styles(const element_t& el) {
    auto& result = fill_styles_;
    result.clear();

    // Special null fill-style
    result.emplace_back(render_op::fill_end);

    for (const auto& fill: el.fills) {
        result.emplace_back(render_op::fill_begin, &fill);
    }
}

void shape_decoder::read_line_styles(const element_t& el) {
    auto& result = line_styles_;
    result.clear();

    // Special null line-style
    result.emplace_back(render_op::line_style_reset);

    for (const auto& stroke: el.strokes) {
        if (stroke.is_solid) {
            result.emplace_back(render_op::line_style_setup, &stroke);
        } else {
            /// TODO: check if not solid stroke
        }
    }
}

void shape_decoder::flush_commands(const std::vector<render_command>& edges, std::vector<shape_edge>& fills) {
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
                    fills.erase(fills.begin() + i);
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
            EK_WARN("Fill Style %d not found", first.fill_style_idx);
            continue;
        }

//          if (!init) {
//              init = true;
        if (current_fill != first.fill_style_idx) {
            commands_.push_back(fill_styles_[first.fill_style_idx]);
            current_fill = first.fill_style_idx;
        }
//          }
        const float2& m = first.p0;

        commands_.emplace_back(render_op::move_to, m);
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
        commands_.emplace_back(render_op::fill_end);
    }

    if (!edges.empty()) {
        //trace("EDGES: " + edges.toString());
        for (const auto& e: edges) {
            commands_.push_back(e);
        }
        commands_.emplace_back(render_op::line_style_reset);
    }
}

void shape_decoder::extend(const float2& p, float r) {
    bounds_builder_.add(p, r);
}

bool shape_decoder::empty() const {
    return bounds_builder_.empty() || total_ == 0;
}

render_batch shape_decoder::result() const {
    render_batch res;
    res.transform = transform_;
    res.bounds = bounds_builder_;
    res.total = total_;
    res.commands = commands_;
    return res;
}

}