#pragma once

#include <ek/math/vec.hpp>
#include <ek/scenex/base/Script.hpp>
#include <ecxx/ecxx.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/timers.hpp>

namespace ek {

template<typename T>
struct VectorDequeue {
    // just storage for use
    uint32_t capacity = 0;
    uint32_t end = 0;

    uint32_t first = 0;
    std::vector<T> data;

    void move() {
        uint32_t dst = 0;
        for(uint32_t i = first; i < end; ++i) {
            data[dst] = data[i];
            ++dst;
        }
        end -= first;
        first = 0;
    }

    inline T& back() {
        assert(first < end);
        return data[end - 1];
    }

    void push_back(T el) {
        if(end == capacity && first > 0) {
            move();
        }
        if(end < capacity) {
            data[end++] = el;
        }
        else {
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
};

struct Trail2D {
    struct Node {
        float2 position{};
        float energy = 1.0f;
        float scale = 1.0f;
    };

    TimeLayer timer;
    float2 offset;
    float drain_speed = 1.0f;
    float segment_distance_max = 10.0f;
//    float particles_per_second = 15.0f;

    Trail2D() = default;

    void update(ecs::Entity owner);

    static void updateAll();

private:

    void update_position(float2 newPosition);

public:
    float scale = 1.0f;
    float2 lastPosition;
    VectorDequeue<Node> nodes;
    bool initialized = false;
};

class TrailRenderer2D : public Drawable2D<TrailRenderer2D> {
public:
    explicit TrailRenderer2D(ecs::entity target_) :
            target{target_} {

    }

    void draw() override;

    [[nodiscard]]
    bool hitTest(float2 point) const override {
        (void)point;
        return false;
    }

    [[nodiscard]]
    rect_f getBounds() const override { return rect_f{}; }

public:
    ecs::entity target{};
    float width = 20.0f;
    float minWidth = 5.0f;
    Res<Sprite> sprite{"empty"};
};

}


