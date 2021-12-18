#pragma once

#include <ek/math/Vec.hpp>
#include <ek/scenex/base/Script.hpp>
#include <ecxx/ecxx.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/time.h>
#include <ek/scenex/types.h>

namespace ek {

template<typename T>
struct VectorDequeue {
    // just storage for use
    uint32_t capacity = 0;
    uint32_t end = 0;

    uint32_t first = 0;
    Array<T> data;

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
};

struct Trail2D {
    struct Node {
        Vec2f position{};
        float energy = 1.0f;
        float scale = 1.0f;
    };

    TimeLayer timer;
    Vec2f offset;
    float drain_speed = 2.0f;
    float segment_distance_max = 10.0f;
//    float particles_per_second = 15.0f;

    Trail2D() = default;

    void update(const Matrix3x2f& m);

    static void updateAll();

private:

    void update_position(Vec2f newPosition);

public:
    float scale = 1.0f;
    Vec2f lastPosition;
    VectorDequeue<Node> nodes;
    bool initialized = false;
};

ECX_TYPE(26, Trail2D);

class TrailRenderer2D : public Drawable2D<TrailRenderer2D> {
public:
    explicit TrailRenderer2D(ecs::EntityApi target_) :
    // TODO: inject world context
            w{&ecs::the_world},
            target{target_} {
    }

    void draw() override;

    [[nodiscard]]
    bool hitTest(Vec2f point) const override {
        (void) point;
        return false;
    }

    [[nodiscard]]
    Rect2f getBounds() const override { return Rect2f{}; }

public:
    ecs::World* w = nullptr;
    ecs::EntityApi target{};
    // max width
    float width = 20.0f;
    float minWidth = 5.0f;
    Res <Sprite> sprite{"empty"};
};

EK_TYPE_INDEX_T(IDrawable2D, TrailRenderer2D, IDrawable2D_TrailRenderer2D);

}


