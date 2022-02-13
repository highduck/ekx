#pragma once

#include <cstring>
#include <cstdint>
#include <ecx/ecx_fwd.hpp>
#include <ek/ds/Array.hpp>
#include <ek/ds/Hash.hpp>

namespace ek {

class RegularGrid {
public:
    irect_t bounds;
    uint32_t cellSize;
    uint32_t power;
    uint32_t columns;
    uint32_t rows;
    uint32_t count;

    Array <entity_t> refs{};
    Array <uint16_t> next{};
    uint16_t* grid;

    Hash <entity_t> filter{};
    Array <entity_t> query{};

    RegularGrid(irect_t bounds_, uint32_t power_) :
            bounds{bounds_},
            cellSize{1u << power_},
            power{power_},
            columns{(bounds_.w + cellSize - 1) / cellSize},
            rows{(bounds_.h + cellSize - 1) / cellSize},
            count{columns * rows} {
        grid = (uint16_t*) calloc(count, sizeof(uint16_t));
        refs.push_back(NULL_ENTITY);
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

    void insert(entity_t entity, const rect_t rc) {
        insert(entity, (irect_t) {{
                                          (int) rc.x,
                                          (int) rc.y,
                                          (int) rc.w,
                                          (int) rc.h
                                  }});
    }

    void insert(entity_t entity, const irect_t rc) {
        if (!irect_overlaps(bounds, rc)) {
            return;
        }

        irect_t clamped = irect_clamp_bounds(rc, bounds);
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

    const Array <entity_t>& search(const rect_t rc) {
        return search((irect_t) {{
                                         (int) rc.x,
                                         (int) rc.y,
                                         (int) rc.w,
                                         (int) rc.h
                                 }});
    }

    const Array <entity_t>& search(const irect_t rc) {
        if (!irect_overlaps(bounds, rc)) {
            return query;
        }
        irect_t clamped = irect_clamp_bounds(rc, bounds);
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
                    if (!filter.has(entity.id)) {
                        query.push_back(entity);
                        filter.set(entity.id, entity);
                    }
                    it = next[it];
                }
            }
        }

        return query;
    }
};

}
