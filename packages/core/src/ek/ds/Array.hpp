#pragma once

#include "../assert.hpp"
#include "../util/Type.hpp"
#include <initializer_list>
#include <cstdlib>
#include <new>
#include <cstdint>

namespace ek {

template<typename T>
inline void constructMove(T* dest, T* src, unsigned count) {
    for (unsigned i = 0; i < count; ++i) {
        new(dest + i)T(static_cast<T&&>(src[i]));
    }
}

template<typename T>
inline void constructCopy(T* dest, const T* src, unsigned count) {
    for (unsigned i = 0; i < count; ++i) {
        new(dest + i)T(static_cast<const T&>(src[i]));
    }
}

template<typename T>
class Array {
public:
    using value_type = T;
    uint32_t _capacity;
    uint32_t _size;
    T* _data;

//    Array() : _capacity{0},
//              _size{0},
//              _data{nullptr} {
//    }

    Array(std::initializer_list<T> list) noexcept: _capacity{static_cast<uint32_t>(list.size())},
                                                   _size{static_cast<uint32_t>(list.size())},
                                                   _data{nullptr} {
        if (_capacity != 0) {
            _data = (T*) malloc(sizeof(T) * _capacity);
            constructCopy(_data, list.begin(), _size);
        }
    }

    explicit Array(unsigned capacity) : _capacity{capacity},
                                        _size{0},
                                        _data{nullptr} {
        if (_capacity != 0) {
            _data = (T*) malloc(sizeof(T) * capacity);
        }
    }

    Array() : Array(0) {
    }

    Array(Array&& m) noexcept: _capacity{m._capacity},
                               _size{m._size},
                               _data{m._data} {
        m._data = nullptr;
    }

    Array(const Array& m) noexcept: _capacity{m._capacity},
                                    _size{m._size},
                                    _data{nullptr} {
        if (_capacity != 0) {
            _data = (T*) malloc(sizeof(T) * _capacity);
            constructCopy(_data, m._data, _size);
        }
    }

    Array& operator=(Array&& m) noexcept {
        clear();
        free(_data);
        _capacity = m._capacity;
        _size = m._size;
        _data = m._data;
        m._data = nullptr;
        return *this;
    }

    Array& operator=(const Array& m) noexcept {
        if (&m == this) {
            return *this;
        }
        clear();
        if (_capacity < m._size) {
            _capacity = m._capacity;
            _size = m._size;
            _data = (T*) realloc(_data, sizeof(T) * _capacity);
        } else {
            if(_size > m._size) {
                reduceSize(m._size);
            }
            // will just copy
            _size = m._size;
        }
        constructCopy(_data, m._data, _size);
        return *this;
    }

    ~Array() {
        if (_data != nullptr) {
            clear();
            free(_data);
        }
    }

    [[nodiscard]]
    inline bool empty() const {
        return _size == 0;
    }

    inline void clear() {
        reduceSize(0);
    }

    void grow(uint32_t capacity) {
        EK_ASSERT_R2(capacity != 0);
        _capacity = capacity;
        T* newPtr = (T*) malloc(sizeof(T) * capacity);
        if (_data) {
            constructMove(newPtr, _data, _size);
            free(_data);
        }
        _data = newPtr;
    }

    void reserve(uint32_t capacity) {
        if (_capacity < capacity) {
            grow(capacity);
        }
    }

    inline void growPow2() {
        grow(_capacity == 0 ? 4 : (_capacity << 1));
    }

    inline void push_back(const T& el) {
        if (_size == _capacity) {
            growPow2();
        }

        // issue `crash on Array::push_back with std::string`
        // *(_data + _size) = el; <- works only for POD structures
        // solution to use copy constructor instead of copy assignment

        T* buff = _data + (_size++);
        // copy constructor
        new(buff)T(el);
    }

    inline T& emplace_back(T&& el) {
        if (_size == _capacity) {
            growPow2();
        }
        // move constructor
        T* buff = _data + (_size++);
        new(buff)T((T&&) (el));
        return *buff;
    }

    template<typename ...Args>
    inline T& emplace_back(Args&& ...args) {
        if (_size == _capacity) {
            growPow2();
        }
        T* buff = _data + _size;
        ++_size;
        new(buff)T(args...);
        return *buff;
    }

    void reduceSize(unsigned smallerSize) {
        EK_ASSERT_R2(smallerSize <= _size);

        for (unsigned i = smallerSize; i < _size; ++i) {
            (_data + i)->~T();
        }

        _size = smallerSize;
    }

    void resize(uint32_t newSize) {
        if (newSize < _size) {
            reduceSize(newSize);
        } else {
            if (newSize > _capacity) {
                // TODO: optimize by nextPowerOfTwo
                grow(newSize);
            }
            // TODO: optimize pre-reserve if no capacity
            for (unsigned i = _size; i < newSize; ++i) {
                T* buff = _data + i;
                new(buff)T();
            }
            _size = newSize;
        }
    }

    inline void eraseIterator(T* el) {
        EK_ASSERT_R2(el != nullptr);
        EK_ASSERT_R2(el >= _data);
        eraseAt(el - _data);
    }

    void eraseAt(unsigned i) {
        EK_ASSERT(i >= 0);
        EK_ASSERT(i < _size);

        (_data + i)->~T();
        // [A, A, A, X, B, B]
        // [A, A, A, B, B]
        --_size;
        constructMove(_data + i, _data + i + 1, _size - i);
    }

    inline void set(unsigned i, const T& el) const {
        EK_ASSERT_R2(i < _size);
        *(_data + i) = el;
    }

    [[nodiscard]]
    inline const T& get(unsigned i) const {
        EK_ASSERT_R2(i < _size);
        return *(_data + i);
    }

    [[nodiscard]]
    inline T& get(unsigned i) {
        EK_ASSERT_R2(i < _size);
        return *(_data + i);
    }

    inline void pop_back() {
        EK_ASSERT_R2(_size > 0);
        --_size;
        (_data + _size)->~T();
    }

    bool remove(const T& el) {
        for (unsigned i = 0; i < _size; ++i) {
            if (el == _data[i]) {
                eraseAt(i);
                return true;
            }
        }
        return false;
    }

    inline void swapRemove(unsigned i) {
        EK_ASSERT_R2(i < _size);
        (_data + i)->~T();
        --_size;
        if (i < _size) {
            constructMove(_data + i, _data + _size, 1);
        }
    }

    inline void swapRemoveFromMiddle(unsigned i) {
        // element to remove SHOULD NOT be at the end of the list,
        // to remove from the end you need another function!
        EK_ASSERT_R2(i < (_size - 1));
        (_data + i)->~T();
        --_size;
        constructMove(_data + i, _data + _size, 1);
    }

    [[nodiscard]]
    inline const T& back() const {
        EK_ASSERT(_size > 0);
        return *(_data + _size - 1);
    }

    [[nodiscard]]
    inline T& back() {
        EK_ASSERT(_size > 0);
        return *(_data + _size - 1);
    }

    [[nodiscard]]
    inline const T& front() const {
        EK_ASSERT(_size > 0);
        return *_data;
    }

    [[nodiscard]]
    inline T& front() {
        EK_ASSERT(_size > 0);
        return *_data;
    }

    inline const T* begin() const {
        return _data;
    }

    inline const T* end() const {
        return _data + _size;
    }

    inline T* begin() {
        return _data;
    }

    inline T* end() {
        return _data + _size;
    }

    inline T& operator[](unsigned i) {
        EK_ASSERT_R2(i < _size);
        return *(_data + i);
    }

    inline const T& operator[](unsigned i) const {
        EK_ASSERT_R2(i < _size);
        return *(_data + i);
    }

    [[nodiscard]]
    inline uint32_t size() const {
        return _size;
    }

    [[nodiscard]]
    inline T* data() {
        return _data;
    }

    [[nodiscard]]
    inline const T* data() const {
        return _data;
    }
};


}