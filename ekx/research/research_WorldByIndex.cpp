#include <cstdint>

// keep size POT
struct World {
    uint32_t pool[0x10000];
    float x[0x10000];
    //float y[0x10000];
    //float z[0x10000];
};

inline static World worlds[10];

void init() {
    for(int i = 0; i < 16; ++i) {
        //worlds[i] = new World();
        for(int j = 0; j < 0x10000; ++j) {
            worlds[i].x[j] = j;
        }
    }
}

struct ObjectWithPointer {
    World* pWorld;
    uint16_t index;

    inline float getX() const {
        return pWorld->x[index];
    }
};

struct ObjectWithIndex {
    // keep worls index first to remove extra shift
    uint8_t world;
    uint8_t generation;
    uint16_t index;

    float getX() const {
        return worlds[world].x[index];
    }

    World* getWorld() const {
        return worlds + world;
    }
};

float test1(ObjectWithPointer o) {
    return o.getX();
}

float test2(ObjectWithIndex o) {
    return o.getX();
}

void* test3(ObjectWithIndex o) {
    return o.getWorld();
}

int main() {
    return sizeof(World);
}