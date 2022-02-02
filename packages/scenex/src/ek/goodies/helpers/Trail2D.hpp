#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/time.h>
#include <ekx/app/time_layers.h>

namespace ek {

template<typename T>
struct VectorDequeue {
    // just storage for use
    uint32_t capacity = 0;
    uint32_t end = 0;

    uint32_t first = 0;
    PodArray<T> data{};

    void move() {
        uint32_t dst = 0;
        for (uint32_t i = first; i < end; ++i) {
            data[dst] = data[i];
            ++dst;
        }
        end -= first;
        first = 0;
    }

    inline T& back() {
        EK_ASSERT(first < end);
        return data[end - 1];
    }

    void push_back(T el) {
        if (end == capacity && first > 0) {
            move();
        }
        if (end < capacity) {
            data[end++] = el;
        } else {
            data.push_back(el);
            ++capacity;
            ++end;
        }
    }

    inline uint32_t size() const {
        return end - first;
    }

    void clear() {
        first = 0;
        end = 0;
    }

    inline void erase_front() {
        ++first;
    }

//    VectorDequeue() {
//        ek_core_dbg_inc(EK_CORE_DBG_VD);
//    }
//
//    ~VectorDequeue() {
//        ek_core_dbg_dec(EK_CORE_DBG_VD);
//    }
};

struct Trail2D {
    struct Node {
        vec2_t position = {};
        float energy = 1.0f;
        float scale = 1.0f;
    };

    TimeLayer timer = 0;
    vec2_t offset = {};
    float drain_speed = 2.0f;
    float segment_distance_max = 10.0f;
//    float particles_per_second = 15.0f;

    //Trail2D(Trail2D&& m) = default;

    void update(mat3x2_t m);

    static void updateAll();

private:

    void update_position(vec2_t newPosition);

public:
    float scale = 1.0f;
    vec2_t lastPosition = {};
    VectorDequeue<Node> nodes{};
    bool initialized = false;
};

class TrailRenderer2D {
public:
    void draw();

public:
    ecs::EntityApi target{};
    // max width
    float width = 20.0f;
    float minWidth = 5.0f;
    R(sprite_t) sprite = R_SPRITE_EMPTY;
};

void trail_renderer2d_draw(entity_t e);

}

ECX_COMP_TYPE_CXX(ek::Trail2D)
