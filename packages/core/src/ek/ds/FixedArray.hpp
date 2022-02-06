#pragma once

#include <ek/assert.h>
#include <initializer_list>

namespace ek {

// TODO: move & copy constructor and assignment
template<typename T, unsigned MaxCount>
struct FixedArray {

    static_assert(MaxCount <= 0x10000);

    unsigned _size;
    T _data[MaxCount];

    FixedArray() noexcept: _size{0}, _data{} {
    }

    FixedArray(std::initializer_list<T> list) noexcept: _size{(uint32_t)list.size()} {
        memcpy(_data, list.begin(), list.size() * sizeof(T));
    }

    constexpr void push_back(T el) {
        EK_ASSERT(_size < MaxCount);
        _data[_size++] = el;
    }

    constexpr void emplace_back(T&& el) {
        EK_ASSERT(_size < MaxCount);
        _data[_size++] = std::move(el);
    }

    constexpr T* begin() const {
        return (T*)_data;
    }

    constexpr T* end() const {
        return (T*)_data + _size;
    }

    [[nodiscard]]
    constexpr unsigned size() const {
        return _size;
    }

    [[nodiscard]]
    constexpr bool empty() const {
        return _size == 0;
    }

    void erase_at(unsigned index) {
        EK_ASSERT(index < size());
        --_size;
        for (unsigned i = index; i < _size; ++i) {
            _data[i] = std::move(_data[i + 1]);
        }
    }

    [[nodiscard]]
    constexpr T* data() const {
        return (T*)_data;
    }

    [[nodiscard]]
    constexpr T back() const {
        EK_ASSERT(_size > 0);
        return *(_data + _size - 1);
    }

    constexpr T& operator[](unsigned i) const {
        EK_ASSERT(i < _size);
        return (T&)_data[i];
    }

    constexpr void clear() {
        _size = 0;
    }

    const T* find(const T& v) const {
        for (uint32_t i = 0; i < _size; ++i) {
            if (_data[i] == v) {
                return _data + i;
            }
        }
        return nullptr;
    }
};

}
