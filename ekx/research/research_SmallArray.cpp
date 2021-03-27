#include <array>

template<typename T, unsigned MaxCount>
struct SmallArray {
    unsigned size = 0;
    T data[MaxCount];

    inline void push(T el) {
        data[size++] = el;
    }
};

template<typename T, unsigned MaxCount>
struct SmallArray2 {
    unsigned size = 0;
    std::array<T, MaxCount> data;

    inline void push(T el) {
        data[size++] = el;
    }
};

struct Data {
    float x;
    float y;
};

int main() {
    SmallArray<Data, 0x10000> positions;
    for(unsigned i = 0; i < 200; ++i) {
        positions.push({(float)i, (float)i + 1});
    }
    float acc = 0.0f;
    for(unsigned i = 0; i < positions.size; ++i) {
        auto p = positions.data[i];

        acc += p.x + p.y;
    }
    return (int)acc;
}