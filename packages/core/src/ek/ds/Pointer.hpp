#pragma once

#include <ek/assert.h>
#include <new>
#include <cstdlib>

namespace ek {

template<typename T>
class Pointer final {
public:
    void* _pointer = nullptr;

    Pointer() = default;

    Pointer(T* pointer) : _pointer{pointer} {
    }

    template<typename ...Args>
    static Pointer<T> make(Args&& ... args) {
        void* ptr = malloc(sizeof(T));
        new(ptr)T(std::forward<Args>(args)...);
        return {(T*) ptr};
    }

    Pointer(Pointer&& other) noexcept {
        _pointer = other.release();
    }

    template<typename U>
    Pointer(Pointer<U>&& other) noexcept {
        _pointer = other.release();
    }

    Pointer(const Pointer& other) = delete;

    Pointer& operator=(Pointer&& other) noexcept {
        _pointer = other.release();
        return *this;
    }

    template<typename U>
    Pointer<T>& operator=(Pointer<U>&& other) noexcept {
        _pointer = other.release();
        return *this;
    }

    Pointer& operator=(const Pointer& other) = delete;

    ~Pointer() {
        if(_pointer) {
            ((T*)_pointer)->~T();
            free(_pointer);
            _pointer = nullptr;
        }
    }

    void reset(T* instance = nullptr) {
        if (_pointer != instance) {
            this->~Pointer();
            _pointer = instance;
        }
    }

    bool operator==(std::nullptr_t) const { return !_pointer; }

    bool operator!=(std::nullptr_t) const { return _pointer; }

    explicit operator bool() const { return _pointer; }

    T* get() { return (T*) _pointer; }

    const T* get() const { return (const T*) _pointer; }

    T* operator->() {
        assertIsValid();
        return (T*) _pointer;
    }

    const T* operator->() const {
        assertIsValid();
        return (const T*) _pointer;
    }

    T& operator*() {
        assertIsValid();
        return *(T*) _pointer;
    }

    const T& operator*() const {
        assertIsValid();
        return *(const T*) _pointer;
    }

    void assertIsValid() const {
        EK_ASSERT(_pointer != nullptr);
    }

    template<typename U, typename ...Args>
    U& emplace(Args&& ... args) {
        this->~Pointer();
        _pointer = malloc(sizeof(U));
        new(_pointer)U(std::forward<Args>(args)...);
        return *((U*) _pointer);
    }

    template<typename ...Args>
    T& emplace(Args&& ... args) {
        return this->emplace<T>(std::forward<Args>(args)...);
    }

    T* release() {
        const auto* out = _pointer;
        _pointer = nullptr;
        return (T*) out;
    }
};

template<class T>
bool operator==(std::nullptr_t, const Pointer<T>& b) { return b == nullptr; }

template<class T>
bool operator!=(std::nullptr_t, const Pointer<T>& b) { return b != nullptr; }

}