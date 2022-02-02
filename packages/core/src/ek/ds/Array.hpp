#pragma once

#include <ek/buf.h>
#include <ek/assert.h>
#include <ek/core_dbg.h>

// for Array
#include <initializer_list>
#include <new>
#include <utility>
#include <type_traits>

namespace ek {

template<typename T, bool Overlap = false>
inline void constructMove(T* dest, T* src, uint32_t count) {
    if constexpr(std::is_trivial_v<T>) {
        if constexpr(Overlap) {
            // for removing holes it's enough,
            for (uint32_t i = 0; i < count; ++i) {
                dest[i] = src[i];
            }
            // TODO: for inserting elements we need to do copy in backward order
//            memmove(dest, src, count * sizeof(T));
        } else {
            memcpy(dest, src, count * sizeof(T));
        }
    } else {
        static_assert(std::is_nothrow_move_constructible<T>::value, "move constructor required");
#pragma nounroll
        for (uint32_t i = 0; i < count; ++i) {
            new(dest + i)T(std::move(src[i]));
            src[i].~T();
        }
    }
}

template<typename T>
inline void constructCopy(T* dest, const T* src, uint32_t count) {
    if constexpr(std::is_trivial_v<T>) {
        memcpy(dest, src, count * sizeof(T));
    } else {
#pragma nounroll
        for (uint32_t i = 0; i < count; ++i) {
            new(dest + i)T(src[i]);
        }
    }
}

template<typename T>
inline void constructDefault(T* begin, T* end) {
//    if constexpr(std::is_trivial_v<T> && !std::is_default_constructible_v<T>) {
//         NOOP
//         TODO: zero init case
//    } else {
    EK_ASSERT_R2(begin <= end);
#pragma nounroll
    for (T* it = begin; it != end; ++it) {
        new(it)T();
    }
//    }
}

template<typename T>
inline void destruct(T* begin, T* end) {
//    if constexpr(std::is_trivial_v<T>) {
        // NOOP
//    } else {
        EK_ASSERT_R2(begin <= end);
#pragma nounroll
        for (T* it = begin; it != end; ++it) {
            it->~T();
        }
//    }
}

template<typename T>
class Array {
public:
    using value_type = T;
    static constexpr uint32_t ElementSize = sizeof(T);
    using SizeType = uint32_t;

    void* buffer = nullptr;

    Array(std::initializer_list<T> list) noexcept: buffer{nullptr} {
        ek_core_dbg_inc(EK_CORE_DBG_ARRAY);
        _initCopyMem(list.begin(), (SizeType) list.size());
        // TODO: test length should be valid after initializer_list
    }

    explicit Array(SizeType capacity) noexcept: buffer{nullptr} {
        ek_core_dbg_inc(EK_CORE_DBG_ARRAY);
        ek_buf_set_capacity(&buffer, capacity, ElementSize);
    }

    /*constexpr*/ Array() noexcept: buffer{nullptr} {
        ek_core_dbg_inc(EK_CORE_DBG_ARRAY);
    }

    /*constexpr*/ Array(Array&& m) noexcept : buffer{m.buffer}{
        ek_core_dbg_inc(EK_CORE_DBG_ARRAY);
        m.buffer = nullptr;
    }

    Array(const Array& m) noexcept: buffer{nullptr} {
        ek_core_dbg_inc(EK_CORE_DBG_ARRAY);
        _initCopyMem((const T*) m.buffer, ek_buf_length(m.buffer));
    }

    void _initCopyMem(const T* ptr, SizeType len) {
        ek_buf_set_size(&buffer, ElementSize, len, len);
        constructCopy((T*) buffer, ptr, len);
    }

    void reset() {
        if (buffer) {
            reduce_size(0);
            ek_buf_reset(&buffer);
        }
    }

    Array& operator=(Array&& m) noexcept {
        if (this == &m) {
            return *this;
        }

        if(buffer == m.buffer) {
            return *this;
        }

        // destruct all prev allocated elements
        // and free previous storage if allocated
        reset();

        buffer = m.buffer;
        m.buffer = nullptr;
        return *this;
    }

    Array& operator=(const Array& m) noexcept {
        // instances are literally the same,
        // also return on no-op empty to empty case

        // TODO: add test for assigning self to self
        if (this == &m) {
            return *this;
        }

        if(buffer == m.buffer) {
            return *this;
        }

        // destruct all prev allocated elements
        reduce_size(0);
        const auto otherSize = ek_buf_length(m.buffer);
        if (ek_buf_capacity(buffer) < otherSize) {
            // grow buffer
            ek_buf_set_capacity(&buffer, ek_buf_capacity(m.buffer), ElementSize);
        }
        if (buffer) {
            ek_buf_header(buffer)->length = otherSize;
            constructCopy(begin(), m.begin(), otherSize);
        }
        return *this;
    }

    ~Array() noexcept {
        ek_core_dbg_dec(EK_CORE_DBG_ARRAY);
        reset();
    }

    [[nodiscard]]
    bool empty() const {
        return ek_buf_empty(buffer);
    }

    void clear() {
        reduce_size(0);
    }

    void grow(SizeType capacity) {
        EK_ASSERT_R2(capacity != 0);
        const auto sz = ek_buf_length(buffer);

        void* newBuffer = nullptr;
        ek_buf_set_size(&newBuffer, ElementSize, sz, capacity);
        constructMove((T*) newBuffer, (T*) buffer, sz);

        ek_buf_reset(&buffer);
        buffer = newBuffer;
    }

    void reserve(SizeType capacity) {
        if (ek_buf_capacity(buffer) < capacity) {
            grow(capacity);
        }
    }

    void maybeGrow() {
        if (ek_buf_full(buffer)) {
            grow(buffer ? (ek_buf_capacity(buffer) << 1) : 1);
        }
    }

    void push_back(const T& el) {
        maybeGrow();

        auto* buff = (T*) ek_buf_add_(buffer, ElementSize);

        // copy constructor
        new(buff)T(el);
    }

    T& emplace_back(T&& el) {
        maybeGrow();

        // move constructor
        auto* buff = (T*) ek_buf_add_(buffer, ElementSize);
        static_assert(std::is_nothrow_move_constructible<T>::value, "move constructor required");
        new(buff)T(std::move(el));
        //el.~T();
        return *buff;
    }

    template<typename ...Args>
    T& emplace_back(Args&& ...args) {
        maybeGrow();

        auto* buff = (T*) ek_buf_add_(buffer, ElementSize);
        new(buff)T(args...);
        return *buff;
    }

    void reduce_size(SizeType smallerSize) {
        // TODO: test reduceSize to 0 for empty buffer
        auto* elements = (T*) buffer;
        if (elements) {
            auto* hdr = ek_buf_header(buffer);
            const auto sz = hdr->length;
            destruct(elements + smallerSize, elements + sz);
            hdr->length = smallerSize;
        }
    }

    void resize(SizeType newSize) {
        const auto len = ek_buf_length(buffer);
        if (newSize < len) {
            reduce_size(newSize);
        } else {
            const auto cap = ek_buf_capacity(buffer);
            if (newSize > cap) {
                // TODO: optimize by nextPowerOfTwo
                grow(newSize);
            }
            // TODO: optimize pre-reserve if no capacity
            if (buffer) {
                T* elements = (T*) buffer;
                constructDefault(elements + len, elements + newSize);
                ek_buf_header(buffer)->length = newSize;
            }
        }
    }

    void eraseIterator(T* el) {
        EK_ASSERT_R2(el != nullptr);
        EK_ASSERT_R2(el >= begin());
        erase_at(el - begin());
    }

    void erase_at(SizeType i) {
        EK_ASSERT(i < size());
//        (begin() + i)->~T();
        // [A, A, A, X, B, B]
        // [A, A, A, B, B]
//        --buffer.header()->length;

        auto* p = (T*) ek_buf_remove_(buffer, i, ElementSize);
        p->~T();
        constructMove<T, true>(p, p + 1, size() - i);
    }

    void set(SizeType i, const T& el) {
        EK_ASSERT_R2(i < size());
        *(begin() + i) = el;
    }

    [[nodiscard]]
    T& get(SizeType i) const {
        EK_ASSERT_R2(i < size());
        return *(begin() + i);
    }

    void pop_back() {
        EK_ASSERT_R2(!empty());
        // TODO: add test
        auto* p = (T*) ek_buf_pop_(buffer, ElementSize);
        p->~T();
    }

    bool remove(const T& el) {
#pragma nounroll
        for (SizeType i = 0; i < size(); ++i) {
            if (el == begin()[i]) {
                erase_at(i);
                return true;
            }
        }
        return false;
    }

    void swap_remove(SizeType i) {
        EK_ASSERT_R2(i < size());
        auto* removedSlot = (T*) ek_buf_remove_(buffer, i, ElementSize);
        removedSlot->~T();
        const auto len = ek_buf_length(buffer);
        if (i < len) {
            constructMove<T>(removedSlot, begin() + len, 1);
        }
    }

    [[nodiscard]]
    T& back() const {
        EK_ASSERT(!empty());
        return *(end() - 1);
    }

    [[nodiscard]]
    T& front() const {
        EK_ASSERT(!empty());
        return *begin();
    }

    T* begin() const {
        return (T*) buffer;
    }

    T* end() const {
        return begin() + size();
    }

    T& operator[](SizeType i) const {
        EK_ASSERT_R2(i < size());
        return *(begin() + i);
    }

    [[nodiscard]]
    SizeType size() const {
        return ek_buf_length(buffer);
    }

    [[nodiscard]]
    SizeType capacity() const {
        return ek_buf_capacity(buffer);
    }

    [[nodiscard]]
    T* data() const {
        return (T*) buffer;
    }

    T* find(const T& v) const {
        const uint32_t total = size();
#pragma nounroll
        for (uint32_t i = 0; i < total; ++i) {
            T* it = (T*) buffer + i;
            if (v == *it) {
                return it;
            }
        }
        return nullptr;
    }
};

}