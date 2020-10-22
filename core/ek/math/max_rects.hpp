#pragma once

#include <vector>
#include <cstdio>
#include <cstdint>
#include <numeric>
#include <limits>
#include <algorithm>

/**
 * Based on [RectangleBinPack](https://github.com/juj/RectangleBinPack) library
 */
namespace ek::binpack {

enum class flags_t : uint8_t {
    empty = 0u,
    packed = 1u << 0u,
    rotated = 1u << 1u
};

constexpr flags_t operator|(flags_t lhs, flags_t rhs) {
    return static_cast<flags_t>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

constexpr flags_t operator&(flags_t lhs, flags_t rhs) {
    return static_cast<flags_t>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

using dim_t = uint16_t;
using area_t = uint32_t;

#pragma pack(push, 1)

struct rect_size_t {
    dim_t width;
    dim_t height;

    [[nodiscard]] inline area_t area() const {
        return width * height;
    }
};

struct rect_t {
    dim_t x;
    dim_t y;
    dim_t width;
    dim_t height;

    [[nodiscard]] inline bool empty() const {
        return !width || !height;
    }

    [[nodiscard]] inline area_t area() const {
        return width * height;
    }

    [[nodiscard]] inline dim_t right() const {
        return x + width;
    }

    [[nodiscard]] inline dim_t bottom() const {
        return y + height;
    }

    [[nodiscard]] inline rect_size_t size() const {
        return {width, height};
    }
};

#pragma pack(pop)

// precondition if 2 rectangles possible to be intersected
inline bool test_separated_axis(const rect_t& a, const rect_t& b) {
    return a.x < b.right() &&
           a.right() > b.x &&
           a.y < b.bottom() &&
           a.bottom() > b.y;
}

inline bool contains(const rect_t& a, const rect_t& b) {
    return a.x <= b.x &&
           a.y <= b.y &&
           a.right() >= b.right() &&
           a.bottom() >= b.bottom();
}

using rect_list = std::vector<rect_t>;
using user_data_list = std::vector<void*>;
using flags_list = std::vector<flags_t>;
using score_t = area_t;

constexpr score_t dim_limit{0x8000u};
constexpr score_t max_score = std::numeric_limits<score_t>::max();

#pragma pack(push, 1)

struct placement_t {
    score_t score1{max_score};
    score_t score2{max_score};
    rect_t rect{};
};

#pragma pack(pop)


inline rect_size_t next_size_power_of_two(rect_size_t size) {
    return {static_cast<dim_t>(size.height << 1u), size.width};
}

inline rect_size_t next_size_step(rect_size_t size) {
    return {static_cast<dim_t>(size.height + dim_t(16)), size.width};
}

inline bool is_size_limit(const rect_size_t size, const rect_size_t max_size) {
    return size.height >= max_size.height && size.width >= max_size.width;
}

inline void push_free_node(rect_list& free, const rect_t& rect) {
    auto end = static_cast<int32_t>(free.size());
    for (int32_t i = 0; i < end; ++i) {
        const auto& free_rect = free[i];
        if (test_separated_axis(rect, free_rect)) {
            if (contains(free_rect, rect)) {
                return;
            }
            if (contains(rect, free_rect)) {
                free.erase(free.begin() + i);
                --end;
                --i;
            }
        }
    }
    free.push_back(rect);
}

inline static void split_free_node(rect_list& out_vec, const rect_t& free_node, const rect_t& used) {
    // Test with SAT if the rectangles even intersect.
    if (!test_separated_axis(used, free_node)) {
        push_free_node(out_vec, free_node);
        return;
    }

    if (used.x < free_node.right() && used.right() > free_node.x) {
        // New node at the top side of the used node.
        if (used.y > free_node.y && used.y < free_node.bottom()) {
            rect_t new_node = free_node;
            new_node.height = used.y - new_node.y;
            push_free_node(out_vec, new_node);
        }

        // New node at the bottom side of the used node.
        if (used.bottom() < free_node.bottom()) {
            rect_t new_node = free_node;
            new_node.y = used.bottom();
            new_node.height = free_node.bottom() - used.bottom();
            push_free_node(out_vec, new_node);
        }
    }

    if (used.y < free_node.bottom() && used.bottom() > free_node.y) {
        // New node at the left side of the used node.
        if (used.x > free_node.x && used.x < free_node.right()) {
            rect_t new_node = free_node;
            new_node.width = used.x - new_node.x;
            push_free_node(out_vec, new_node);
        }

        // New node at the right side of the used node.
        if (used.right() < free_node.right()) {
            rect_t new_node = free_node;
            new_node.x = used.right();
            new_node.width = free_node.right() - used.right();
            push_free_node(out_vec, new_node);
        }
    }
}

struct max_rects_t {
    rect_list free_rects{};
    rect_list used_rects{};
    rect_t target{};

    void resize(rect_size_t size) {
        target = {0u, 0u, size.width, size.height};
    }

    void reset() {
        used_rects.clear();
        free_rects.clear();
        free_rects.emplace_back(target);
    }

    template<typename Method, bool AllowFlip>
    bool choose(placement_t& best, const dim_t width, const dim_t height) const {
        const Method fn(*this);
        bool matched = false;
        for (const auto& free_rect : free_rects) {
            if (free_rect.width >= width &&
                free_rect.height >= height) {
                fn.place(best, free_rect, width, height, matched);
            }
            if constexpr (AllowFlip) {
                if (free_rect.width >= height &&
                    free_rect.height >= width) {
                    fn.place(best, free_rect, height, width, matched);
                }
            }
        }
        return matched;
    }

    /// Places the given rectangle into the bin.
    void place(const rect_t& node) {
        rect_list next_free_list;
        next_free_list.reserve(free_rects.capacity());
        for (const auto& free_rect : free_rects) {
            split_free_node(next_free_list, free_rect, node);
        }
        free_rects = next_free_list;
        used_rects.push_back(node);
    }
};

namespace methods {
struct bottom_left {

    explicit bottom_left(const max_rects_t&) {}

    void place(placement_t& best, const rect_t& free_rect,
               const dim_t width,
               const dim_t height, bool& matched) const {
        const score_t top_side = static_cast<score_t>(height) + free_rect.y;
        if (top_side < best.score1 || (top_side == best.score1 && free_rect.x < best.score2)) {
            best.score1 = top_side;
            best.score2 = free_rect.x;
            best.rect.x = free_rect.x;
            best.rect.y = free_rect.y;
            best.rect.width = width;
            best.rect.height = height;
            matched = true;
        }
    }
};

struct best_short_side_fit {
    explicit best_short_side_fit(const max_rects_t&) {}

    void place(placement_t& best, const rect_t& free_rect, dim_t width, dim_t height, bool& matched) const {

        const dim_t leftover_horiz = free_rect.width - width;
        const dim_t leftover_vert = free_rect.height - height;
        const dim_t short_side_fit = std::min(leftover_horiz, leftover_vert);
        const dim_t long_side_fit = std::max(leftover_horiz, leftover_vert);

        if (short_side_fit < best.score1 ||
            (short_side_fit == best.score1 && long_side_fit < best.score2)) {
            best.score1 = short_side_fit;
            best.score2 = long_side_fit;
            best.rect.x = free_rect.x;
            best.rect.y = free_rect.y;
            best.rect.width = width;
            best.rect.height = height;
            matched = true;
        }
    }
};

struct best_long_side_fit {

    explicit best_long_side_fit(const max_rects_t&) {}

    void place(placement_t& best, const rect_t& free_rect,
               dim_t width,
               dim_t height, bool& matched) const {
        const dim_t leftover_horiz = free_rect.width - width;
        const dim_t leftover_vert = free_rect.height - height;
        const dim_t short_side_fit = std::min(leftover_horiz, leftover_vert);
        const dim_t long_side_fit = std::max(leftover_horiz, leftover_vert);

        if (long_side_fit < best.score2 ||
            (long_side_fit == best.score2 && short_side_fit < best.score1)) {
            best.score1 = short_side_fit;
            best.score2 = long_side_fit;
            best.rect.x = free_rect.x;
            best.rect.y = free_rect.y;
            best.rect.width = width;
            best.rect.height = height;
            matched = true;
        }
    }
};

struct best_area_fit {
    explicit best_area_fit(const max_rects_t&) {}

    inline void place(placement_t& best, const rect_t& free_rect,
                      const dim_t width,
                      const dim_t height, bool& matched) const {
        // opt: don't ABS: free rect is already bigger than {width,height}
        const dim_t space_x = free_rect.width - width;
        const dim_t space_y = free_rect.height - height;
        const dim_t short_side_fit = std::min(space_x, space_y);
        const score_t area_fit = free_rect.area() - width * height;

        if (area_fit < best.score2 || (area_fit == best.score2 && short_side_fit < best.score1)) {
            best.score1 = short_side_fit;
            best.score2 = area_fit;
            best.rect.x = free_rect.x;
            best.rect.y = free_rect.y;
            best.rect.width = width;
            best.rect.height = height;
            matched = true;
        }
    }
};

struct contact_point {
    const rect_list& used_rects;
    const rect_t& target;

    explicit contact_point(const max_rects_t& max_rects) :
            used_rects{max_rects.used_rects},
            target{max_rects.target} {}

    /// Returns 0 if the two intervals i1 and i2 are disjoint, or the length of their overlap otherwise.
    inline static dim_t common_interval_length(
            dim_t first_begin,
            dim_t first_end,
            dim_t second_begin,
            dim_t second_end) {
        if (first_end < second_begin || second_end < first_begin) {
            return 0;
        }
        return std::min(first_end, second_end) - std::max(first_begin, second_begin);
    }

    [[nodiscard]]
    inline score_t contact_point_score_node(const rect_t& rect) const {
        score_t score{max_score};

        if (rect.x == target.x || rect.right() == target.right()) {
            score -= rect.height;
        }

        if (rect.y == target.y || rect.bottom() == target.bottom()) {
            score -= rect.width;
        }

        for (const auto& used_rect : used_rects) {
            if (used_rect.x == rect.right() || used_rect.right() == rect.x) {
                score -= common_interval_length(used_rect.y, used_rect.bottom(), rect.y, rect.bottom());
            }
            if (used_rect.y == rect.bottom() || used_rect.bottom() == rect.y) {
                score -= common_interval_length(used_rect.x, used_rect.right(), rect.x, rect.right());
            }
        }
        return score;
    }

    void place(placement_t& best,
               const rect_t& free_rect,
               dim_t width, dim_t height,
               bool& matched) const {
        const rect_t rc{free_rect.x, free_rect.y, width, height};
        const score_t score = contact_point_score_node(rc);
        if (score < best.score1) {
            best.score1 = score;
            best.rect = rc;
            matched = true;
        }
    }
};
}

struct packer_state_t {
    max_rects_t max_rects;
    rect_size_t canvas{0, 0};
    rect_size_t max_size{dim_limit, dim_limit};
    rect_list rects;
    flags_list flags;
    user_data_list user_data;

    explicit packer_state_t(int max_width = dim_limit, int max_height = dim_limit)
            : max_size{static_cast<dim_t>(max_width), static_cast<dim_t>(max_height)} {

    }

    bool add(int width, int height, int padding, void* user_data_) {
        // filter empty input as well
        if (width > 0 && height > 0) {
            const dim_t w = static_cast<dim_t>(std::max(0, width + padding * 2));
            const dim_t h = static_cast<dim_t>(std::max(0, height + padding * 2));
            // filter ultra-big entries
            if (w <= max_size.width && h <= max_size.height) {
                rects.push_back({0u, 0u, w, h});
                user_data.emplace_back(user_data_);
                flags.emplace_back(flags_t::empty);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]]
    size_t size() const {
        return rects.size();
    }

    [[nodiscard]]
    bool empty() const {
        return rects.empty();
    }

    template<typename T>
    [[nodiscard]]
    inline T& get_user_data(int i) const {
        return *static_cast<T*>(user_data[i]);
    }

    [[nodiscard]]
    inline rect_t get_rect(int i) const {
        return rects[i];
    }

    [[nodiscard]]
    inline flags_t get_flags(int i) const {
        return flags[i];
    }

    [[nodiscard]]
    inline bool is_packed(int i) const {
        return (flags[i] & flags_t::packed) != flags_t::empty;
    }

    [[nodiscard]]
    inline bool is_rotated(int i) const {
        return (flags[i] & flags_t::rotated) != flags_t::empty;
    }
};

// input sizes
// output: rects & indices to original
template<typename Method, bool AllowFlip>
static bool try_pack(max_rects_t& max_rects,
                     rect_list& rects,
                     flags_list& flags_list) {

    max_rects.reset();
    std::fill(flags_list.begin(), flags_list.end(), flags_t::empty);

    const size_t total = rects.size();
    for (size_t j = 0; j < total; ++j) {
        placement_t best{};
        int best_rect_index = -1;
        for (uint32_t i = 0; i < total; ++i) {
            if ((flags_list[i] & flags_t::packed) == flags_t::empty) {
                const auto& rect = rects[i];
                if (max_rects.choose<Method, AllowFlip>(best, rect.width, rect.height)) {
                    best_rect_index = i;
                }
            }
        }

        if (best_rect_index == -1) {
            return false;
        }

        max_rects.place(best.rect);

        auto& rect = rects[best_rect_index];
        rect.x = best.rect.x;
        rect.y = best.rect.y;

        flags_t flags = flags_list[best_rect_index] | flags_t::packed;
        if (rect.width != best.rect.width) {
            flags = flags | flags_t::rotated;
        }
        flags_list[best_rect_index] = flags;
    }

    return true;
}

bool try_pack(packer_state_t& state);

bool pack_nodes(packer_state_t& state);

}