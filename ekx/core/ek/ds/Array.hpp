#pragma once

#include "../Allocator.hpp"
#include "../assert.hpp"

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
struct Array {

    Allocator& _allocator;
    uint32_t _capacity;
    uint32_t _size;
    T* _data;

    Array() : _allocator{memory::stdAllocator},
              _capacity{64},
              _size{0} {
        _data = (T*) _allocator.alloc(sizeof(T) * _capacity, alignof(T));
    }

    explicit Array(Allocator& allocator, unsigned capacity = 64) :
            _allocator{allocator} {
        _size = 0;
        _capacity = capacity;
        _data = (T*) _allocator.alloc(sizeof(T) * capacity, alignof(T));
    }

    Array(Array&& m) noexcept: _allocator{m._allocator},
                               _capacity{m._capacity},
                               _size{m._size},
                               _data{m._data} {
        m._data = nullptr;
    }

    Array(const Array& m) noexcept: _allocator{m._allocator},
                                    _size{m._size},
                                    _capacity{m._capacity} {
        _data = (T*) _allocator.alloc(sizeof(T) * _capacity, alignof(T));
        constructCopy(_data, m._data, _size);
    }

    Array& operator=(Array&& m) noexcept {
        EK_ASSERT_R2(_data != nullptr);
        _allocator.dealloc(_data);
        _allocator = m._allocator;
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
        if (_capacity < m._size) {
            _allocator.dealloc(_data);
            _capacity = m._capacity;
            _size = m._size;
            _data = (T*) _allocator.alloc(sizeof(T) * _capacity, alignof(T));
        } else {
            // will just copy
            _size = m._size;
        }
        constructCopy(_data, m._data, _size);
        return *this;
    }

    ~Array() {
        if (_data != nullptr) {
            clear();
            _allocator.dealloc(_data);
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
        EK_ASSERT_R2(_data != nullptr);
        _capacity = capacity;
        T* newPtr = (T*) _allocator.alloc(sizeof(T) * capacity, alignof(T));
        constructMove(newPtr, _data, _size);
        _allocator.dealloc(_data);
        _data = newPtr;
    }

    void reserve(uint32_t capacity) {
        if(_capacity < capacity) {
            grow(capacity);
        }
    }

    inline void growPow2() {
        grow(_capacity << 1);
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
            if(newSize > _capacity) {
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

    inline void eraseIt(T* el) {
        EK_ASSERT_R2(el != nullptr);
        EK_ASSERT_R2(el >= _data);
        erase(el - _data);
    }

    void erase(unsigned i) {
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

    inline void remove_swap_back(unsigned i) {
        EK_ASSERT_R2(i != _size - 1);
        (_data + i)->~T();
        --_size;
        constructMove(_data + i, _data + _size, 1);
    }

    [[nodiscard]]
    inline T back() const {
        EK_ASSERT_R2(_size > 0);
        return *(_data + _size - 1);
    }

    inline const T* begin() const {
        return _data;
    }

    inline const T* end() const {
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
};

}