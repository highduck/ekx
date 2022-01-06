#pragma once

#include <cstring>
#include <cstdint>
#include <ecxx/ecxx_fwd.hpp>
#include <ek/ds/Array.hpp>
#include <ek/ds/Hash.hpp>

namespace ek {

class RegularGrid {
public:
    recti_t bounds;
    uint32_t cellSize;
    uint32_t power;
    uint32_t columns;
    uint32_t rows;
    uint32_t count;

    Array <ecs::EntityIndex> refs{};
    Array <uint16_t> next{};
    uint16_t* grid;

    Hash <ecs::EntityIndex> filter{};
    Array <ecs::EntityIndex> query{};

    RegularGrid(recti_t bounds_, uint32_t power_) :
            bounds{bounds_},
            cellSize{1u << power_},
            power{power_},
            columns{(bounds_.w + cellSize - 1) / cellSize},
            rows{(bounds_.h + cellSize - 1) / cellSize},
            count{columns * rows} {
        grid = (uint16_t*) calloc(count, sizeof(uint16_t));
        refs.push_back(0);
        next.push_back(0);
    }

    ~RegularGrid() {
        free(grid);
    }

    void reset() {
        const auto size = (uint32_t) sizeof(uint16_t) * count;
        memset(grid, 0, size);
        refs.resize(1);
        next.resize(1);
    }

    void insert(ecs::EntityIndex entity, const rect_t rc) {
        insert(entity, (recti_t) {{
                                          (int) rc.x,
                                          (int) rc.y,
                                          (int) rc.w,
                                          (int) rc.h
                                  }});
    }

    void insert(ecs::EntityIndex entity, const recti_t rc) {
        if (!recti_overlaps(bounds, rc)) {
            return;
        }

        recti_t clamped = recti_clamp_bounds(rc, bounds);
        if (clamped.w <= 0 || clamped.h <= 0) {
            return;
        }
        --clamped.w;
        --clamped.h;
        uint32_t x0 = (clamped.x - bounds.x) >> power;
        uint32_t y0 = (clamped.y - bounds.y) >> power;
        uint32_t x1 = (RECT_R(clamped) - bounds.x) >> power;
        uint32_t y1 = (RECT_B(clamped) - bounds.y) >> power;

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

    const Array <ecs::EntityIndex>& search(const rect_t rc) {
        return search((recti_t) {{
                                         (int) rc.x,
                                         (int) rc.y,
                                         (int) rc.w,
                                         (int) rc.h
                                 }});
    }

    const Array <ecs::EntityIndex>& search(const recti_t rc) {
        if (!recti_overlaps(bounds, rc)) {
            return query;
        }
        recti_t clamped = recti_clamp_bounds(rc, bounds);
        if (clamped.w <= 0 || clamped.h <= 0) {
            return query;
        }
        --clamped.w;
        --clamped.h;

        uint32_t x0 = (clamped.x - bounds.x) >> power;
        uint32_t y0 = (clamped.y - bounds.y) >> power;
        uint32_t x1 = (RECT_R(clamped) - bounds.x) >> power;
        uint32_t y1 = (RECT_B(clamped) - bounds.y) >> power;

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
