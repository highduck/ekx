#pragma once

#include <cstring>
#include <cstdint>
#include <ecxx/ecxx_fwd.hpp>
#include <ek/math/box.hpp>
#include <ek/ds/Array.hpp>
#include <ek/ds/Hash.hpp>

namespace ek {

class RegularGrid {
public:
    rect_i bounds;
    uint32_t cellSize;
    uint32_t power;
    uint32_t columns;
    uint32_t rows;
    uint32_t count;

    Array<ecs::EntityIndex> refs{};
    Array<uint16_t> next{};
    uint16_t* grid;

    Hash<ecs::EntityIndex> filter{};
    Array<ecs::EntityIndex> query{};

    RegularGrid(rect_i bounds_, uint32_t power_) :
            bounds{bounds_},
            cellSize{1u << power_},
            power{power_},
            columns{(bounds_.width + cellSize - 1) / cellSize},
            rows{(bounds_.height + cellSize - 1) / cellSize},
            count{columns * rows} {
        grid = (uint16_t*)calloc(count, sizeof(uint16_t));
        refs.push_back(0);
        next.push_back(0);
    }

    ~RegularGrid() {
        free(grid);
    }

    void reset() {
        const auto size = (uint32_t)sizeof(uint16_t) * count;
        memset(grid, 0, size);
        refs.resize(1);
        next.resize(1);
    }

    void insert(ecs::EntityIndex entity, const rect_f& rc) {
        insert(entity, rect_i{rc});
    }

    void insert(ecs::EntityIndex entity, const rect_i& rc) {
        if(!bounds.overlaps(rc)) {
            return;
        }

        auto clamped = clamp_bounds(rc, bounds);
        if(clamped.width <= 0 || clamped.height <= 0) {
            return;
        }
        clamped.width--;
        clamped.height--;
        uint32_t x0 = (clamped.x - bounds.x) >> power;
        uint32_t y0 = (clamped.y - bounds.y) >> power;
        uint32_t x1 = (clamped.right() - bounds.x) >> power;
        uint32_t y1 = (clamped.bottom() - bounds.y) >> power;

        for (uint32_t cy = y0; cy <= y1; ++cy) {
            for (uint32_t cx = x0; cx <= x1; ++cx) {
                const auto index = cy * columns + cx;
                const auto prevId = grid[index];
                const auto id = refs.size();
                refs.push_back(entity);
                next.push_back(prevId);
                grid[index] = id;
            }
        }
    }

    void beginSearch() {
        filter.clear();
        query.clear();
    }

    const Array<ecs::EntityIndex>& search(const rect_f& rc) {
        return search(rect_i{rc});
    }

    const Array<ecs::EntityIndex>& search(const rect_i& rc) {
        if(!bounds.overlaps(rc)) {
            return query;
        }
        auto clamped = clamp_bounds(rc, bounds);
        if(clamped.width <= 0 || clamped.height <= 0) {
            return query;
        }
        clamped.width--;
        clamped.height--;

        uint32_t x0 = (clamped.x - bounds.x) >> power;
        uint32_t y0 = (clamped.y - bounds.y) >> power;
        uint32_t x1 = (clamped.right() - bounds.x) >> power;
        uint32_t y1 = (clamped.bottom() - bounds.y) >> power;

        for (uint32_t cy = y0; cy <= y1; ++cy) {
            for (uint32_t cx = x0; cx <= x1; ++cx) {
                const auto index = cy * columns + cx;
                auto it = grid[index];
                while (it != 0) {
                    const auto entity = refs[it];
                    if (!filter.has(entity)) {
                        query.push_back(entity);
                        filter.set(entity, entity);
                    }
                    it = next[it];
                }
            }
        }

        return query;
    }
};

}
