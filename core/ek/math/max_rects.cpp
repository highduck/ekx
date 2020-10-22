#include "max_rects.hpp"

namespace ek::binpack {

constexpr dim_t pack_start_size{32};

uint32_t get_area(const rect_list& rects) {
    return std::accumulate(
            rects.cbegin(),
            rects.cend(),
            area_t{0},
            [](area_t acc, const rect_t& r) -> auto {
                return acc + r.area();
            }
    );
}

rect_size_t estimate_size(const rect_size_t start_size, const area_t area, const rect_size_t max_size) {
    rect_size_t size = start_size;
    while (area > size.area() && !is_size_limit(size, max_size)) {
        size = next_size_power_of_two(size);
    }
    return size;
}

bool pack_nodes(packer_state_t& state) {
    state.canvas = estimate_size({pack_start_size, pack_start_size}, get_area(state.rects), state.max_size);

    //LOG_PERF("Start pack |%u| nodes to |%u %u|", nodes.size(), res.width, res.height);
    while (!try_pack(state)) {
        if (is_size_limit(state.canvas, state.max_size)) {
            return false;
        }
        state.canvas = next_size_power_of_two(state.canvas);
        //LOG_PERF("Enlarge to %u %u", res.width, res.height);
    }
    return true;
}

bool try_pack(packer_state_t& state) {
    auto& rects = state.rects;
    auto& flags = state.flags;

    max_rects_t& max_rects = state.max_rects;
    max_rects.resize(state.canvas);

    if (try_pack<methods::best_area_fit, true>(max_rects, rects, flags)) {
        return true;
    }
    if (try_pack<methods::contact_point, true>(max_rects, rects, flags)) {
        return true;
    }
    if (try_pack<methods::bottom_left, true>(max_rects, rects, flags)) {
        return true;
    }
    if (try_pack<methods::best_long_side_fit, true>(max_rects, rects, flags)) {
        return true;
    }
    return try_pack<methods::best_short_side_fit, true>(max_rects, rects, flags);
}
}