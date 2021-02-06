#include <malloc.h>

template<typename T>
struct DynArray {
    unsigned capacity;
    unsigned size;
    T* data;

    DynArray() {
        size = 0;
        capacity = 64;
        data = (T*)malloc(sizeof(T) * capacity);
    }

    ~DynArray() {
        free(data);
    }

    void reset() {
        size = 0;
    }

    void grow() {
        capacity = capacity << 1;
        data = (T*)realloc(data, sizeof(T) * capacity);
    }

    inline void push(T el) {
        if (size == capacity) {
            grow();
        }
        *(data + size) = el;
        ++size;
    }

    inline void set(unsigned i, T el) const {
        *(data + i) = el;
    }

    [[nodiscard]]
    inline T get(unsigned i) const {
        return *(data + i);
    }

    inline void remove_back() {
        --size;
    }

    [[nodiscard]]
    inline T back() const {
        return *(data + size - 1);
    }
};