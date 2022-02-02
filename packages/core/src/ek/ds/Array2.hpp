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

template<typename T>
inline void constructMove(void* dest, const void* src, uint32_t count) {
    if constexpr(std::is_trivial_v<T>) {
        memcpy(dest, src, count * sizeof(T));
    } else {
        static_assert(std::is_nothrow_move_constructible<T>::value, "move constructor required");
        T* p_dest = (T*) dest;
        T* p_src = (T*) src;
#pragma nounroll
        for (uint32_t i = 0; i < count; ++i) {
            T* src_el = p_src + i;
            new(p_dest + i)T(std::move(*src_el));
            src_el->~T();
        }
    }
}


template<typename T>
inline void construct_move_inside(void* buffer, uint32_t dest_i, uint32_t src_i, uint32_t count) {
    if constexpr(std::is_trivial_v<T>) {
        // for removing holes it's enough,
        for (uint32_t i = 0; i < count; ++i) {
            ((T*) buffer + dest_i)[i] = ((T*) buffer + src_i)[i];
        }
        // TODO: for inserting elements we need to do copy in backward order
//            memmove(dest, src, count * sizeof(T));
    } else {
        static_assert(std::is_nothrow_move_constructible<T>::value, "move constructor required");
        T* p_dest = (T*) buffer + dest_i;
        T* p_src = (T*) buffer + src_i;
#pragma nounroll
        for (uint32_t i = 0; i < count; ++i) {
            T* src_el = p_src + i;
            new(p_dest + i)T(std::move(*src_el));
            src_el->~T();
        }
    }
}

template<typename T>
inline void constructCopy(void* dest, const void* src, uint32_t count) {
    if constexpr(std::is_trivial_v<T>) {
        memcpy(dest, src, count * sizeof(T));
    } else if constexpr(std::is_copy_constructible_v<T>) {
        T* p_dest = (T*) dest;
        T* p_src = (T*) src;
#pragma nounroll
        for (uint32_t i = 0; i < count; ++i) {
            new(p_dest + i)T((const T&)p_src[i]);
        }
    }
    else if constexpr(std::is_copy_assignable_v<T>){
        T* p_dest = (T*) dest;
        T* p_src = (T*) src;
#pragma nounroll
        for (uint32_t i = 0; i < count; ++i) {
            *(p_dest++) = *(p_src++);
        }
    }
    else {
        EK_ASSERT(false);
    }
}

template<typename T>
inline void constructDefault(void* buffer, uint32_t begin, uint32_t end) {
//    if constexpr(std::is_trivial_v<T> && !std::is_default_constructible_v<T>) {
//         NOOP
//         TODO: zero init case
//    } else {
    EK_ASSERT_R2(begin <= end);
    T* p_begin = (T*) buffer + begin;
    T* p_end = (T*) buffer + end;
#pragma nounroll
    for (T* it = p_begin; it != p_end; ++it) {
        new(it)T();
    }
//    }
}

template<typename T>
inline void destruct(void* buffer, uint32_t begin, uint32_t end) {
//    if constexpr(std::is_trivial_v<T>) {
    // NOOP
//    } else {
    T* p_buffer = (T*) buffer;
    EK_ASSERT_R2(begin <= end);
    T* p_begin = p_buffer + begin;
    T* p_end = p_buffer + end;
#pragma nounroll
    for (T* it = p_begin; it != p_end; ++it) {
        it->~T();
    }
//    }
}

struct ArrayType {
    void (* destruct)(void* buffer, uint32_t begin, uint32_t end);

    void (* construct_copy)(void* dest, const void* src, uint32_t count);

    void (* construct_move)(void* dest, const void* src, uint32_t count);

    void (* construct_def)(void* buffer, uint32_t begin, uint32_t end);

    void (* construct_move_inside)(void* buffer, uint32_t dest_i, uint32_t src_i, uint32_t count);

    uint32_t size;

//    constexpr ArrayType() noexcept {
//
//    }
};

template<typename T>
struct ArrayTypes {
    constexpr static ArrayType create() noexcept {
        return ArrayType{
                ek::destruct<T>,
                ek::constructCopy<T>,
                ek::constructMove<T>,
                ek::constructDefault<T>,
                ek::construct_move_inside<T>,
                sizeof(T)
        };
    }

    inline static ArrayType value{ArrayTypes<T>::create()};
};

//template<typename T>
//constexpr ArrayType ArrayTypes<T>::value{ArrayTypes<T>::create()};

class ArrayBase {
public:
    void* buffer = nullptr;
    ArrayType& type;

    ArrayBase(void* buffer_, ArrayType& type_): buffer{buffer_}, type{type_} {

    }

    void _initCopyMem(const void* ptr, uint32_t len) {
        EK_ASSERT(type.size);
        ek_buf_set_size(&buffer, type.size, len, len);
        type.construct_copy(buffer, ptr, len);
    }

    void reset() {
        EK_ASSERT(type.size);
        if (buffer) {
            reduce_size(0);
            ek_buf_reset(&buffer);
        }
    }

//
//    Array& operator=(Array&& m) noexcept {
//        if (this == &m) {
//            return *this;
//        }
//
//        if (buffer == m.buffer) {
//            return *this;
//        }
//
//        // destruct all prev allocated elements
//        // and free previous storage if allocated
//        reset();
//
//        buffer = m.buffer;
//        m.buffer = nullptr;
//        return *this;
//    }
//
//    Array& operator=(const Array& m) noexcept {
//        // instances are literally the same,
//        // also return on no-op empty to empty case
//
//        // TODO: add test for assigning self to self
//        if (this == &m) {
//            return *this;
//        }
//
//        if (buffer == m.buffer) {
//            return *this;
//        }
//
//        // destruct all prev allocated elements
//        reduceSize(0);
//        const auto otherSize = ek_buf_length(m.buffer);
//        if (ek_buf_capacity(buffer) < otherSize) {
//            // grow buffer
//            ek_buf_set_capacity(&buffer, ek_buf_capacity(m.buffer), ElementSize);
//        }
//        if (buffer) {
//            ek_buf_header(buffer)->length = otherSize;
//            constructCopy(begin(), m.begin(), otherSize);
//        }
//        return *this;
//    }
//
    ~ArrayBase() noexcept {
        ek_core_dbg_dec(EK_CORE_DBG_ARRAY);
        reset();
    }

    [[nodiscard]]
    bool empty() const {
        EK_ASSERT(type.size);
        return ek_buf_empty(buffer);
    }

    void clear() {
        EK_ASSERT(type.size);
        reduce_size(0);
    }

    void grow(uint32_t capacity) {
        EK_ASSERT(type.size);
        EK_ASSERT_R2(capacity != 0);
        const auto sz = ek_buf_length(buffer);

        void* newBuffer = nullptr;
        ek_buf_set_size(&newBuffer, type.size, sz, capacity);
        type.construct_move(newBuffer, buffer, sz);

        ek_buf_reset(&buffer);
        buffer = newBuffer;
    }

    void reserve(uint32_t capacity) {
        EK_ASSERT(type.size);
        if (ek_buf_capacity(buffer) < capacity) {
            grow(capacity);
        }
    }

    void maybe_grow() {
        EK_ASSERT(type.size);
        if (ek_buf_full(buffer)) {
            grow(buffer ? (ek_buf_capacity(buffer) << 1) : 1);
        }
    }

//
//    void push_back(const T& el) {
//        maybeGrow();
//
//        auto* buff = (T*) ek_buf_add_(buffer, ElementSize);
//
//        // copy constructor
//        new(buff)T(el);
//    }
//
//    T& emplace_back(T&& el) {
//        maybeGrow();
//
//        // move constructor
//        auto* buff = (T*) ek_buf_add_(buffer, ElementSize);
//        static_assert(std::is_nothrow_move_constructible<T>::value, "move constructor required");
//        new(buff)T(std::move(el));
//        //el.~T();
//        return *buff;
//    }
//
//    template<typename ...Args>
//    T& emplace_back(Args&& ...args) {
//        maybeGrow();
//
//        auto* buff = (T*) ek_buf_add_(buffer, ElementSize);
//        new(buff)T(args...);
//        return *buff;
//    }
//
    void reduce_size(uint32_t smaller_size) {
        EK_ASSERT(type.size);
        // TODO: test reduceSize to 0 for empty buffer
        if (buffer) {
            auto* hdr = ek_buf_header(buffer);
            type.destruct(buffer, smaller_size, hdr->length);
            hdr->length = smaller_size;
        }
    }

    void resize(uint32_t new_size) {
        const auto len = ek_buf_length(buffer);
        if (new_size < len) {
            reduce_size(new_size);
        } else {
            const auto cap = ek_buf_capacity(buffer);
            if (new_size > cap) {
                // TODO: optimize by nextPowerOfTwo
                grow(new_size);
            }
            // TODO: optimize pre-reserve if no capacity
            if (buffer) {
                type.construct_def(buffer, len, new_size);
                ek_buf_header(buffer)->length = new_size;
            }
        }
    }

//
//    void eraseIterator(T* el) {
//        EK_ASSERT_R2(el != nullptr);
//        EK_ASSERT_R2(el >= begin());
//        eraseAt(el - begin());
//    }
//
    void erase_at(uint32_t i) {
        EK_ASSERT(i < size());
//        (begin() + i)->~T();
        // [A, A, A, X, B, B]
        // [A, A, A, B, B]
//        --buffer.header()->length;

        ek_buf_remove_(buffer, i, type.size);
        type.destruct(buffer, i, i + 1);
        type.construct_move_inside(buffer, i, i + 1, size() - i - 1);
    }

//    void set(SizeType i, const T& el) {
//        EK_ASSERT_R2(i < size());
//        *(begin() + i) = el;
//    }
//
//    [[nodiscard]]
//    T& get(SizeType i) const {
//        EK_ASSERT_R2(i < size());
//        return *(begin() + i);
//    }
//
    void pop_back() {
        EK_ASSERT_R2(!empty());
        EK_ASSERT(type.size);
        // TODO: add test, optimize
        void* p = (void*) ek_buf_pop_(buffer, type.size);
        type.destruct(p, 0, 1);
    }

//    bool remove(const T& el) {
//#pragma nounroll
//        for (SizeType i = 0; i < size(); ++i) {
//            if (el == begin()[i]) {
//                eraseAt(i);
//                return true;
//            }
//        }
//        return false;
//    }
//
    void swap_remove(uint32_t i) {
        EK_ASSERT(type.size);
        EK_ASSERT_R2(i < size());
        ek_buf_remove_(buffer, i, type.size);
        type.destruct(buffer, i, i + 1);
        const auto len = ek_buf_length(buffer);
        if (i < len) {
            type.construct_move_inside(buffer, i, len, 1);
        }
    }

    void swap_remove_from_middle(uint32_t i) {
        EK_ASSERT(type.size);
        // element to remove SHOULD NOT be at the end of the list,
        // to remove from the end you need another function!
        EK_ASSERT_R2(i < (size() - 1));
        ek_buf_remove_(buffer, i, type.size);
        type.destruct(buffer, i, i + 1);
        type.construct_move_inside(buffer, i, size() - 1, 1);
    }
//
//    [[nodiscard]]
//    T& back() const {
//        EK_ASSERT(!empty());
//        return *(end() - 1);
//    }
//
//    [[nodiscard]]
//    T& front() const {
//        EK_ASSERT(!empty());
//        return *begin();
//    }
//
//    T* begin() const {
//        return (T*) buffer;
//    }
//
//    T* end() const {
//        return begin() + size();
//    }
//
//    T& operator[](SizeType i) const {
//        EK_ASSERT_R2(i < size());
//        return *(begin() + i);
//    }
//
    [[nodiscard]]
    uint32_t size() const {
        return ek_buf_length(buffer);
    }

    [[nodiscard]]
    uint32_t capacity() const {
        EK_ASSERT(type.size);
        return ek_buf_capacity(buffer);
    }

//    [[nodiscard]]
//    T* data() const {
//        return (T*) buffer;
//    }
//
//    T* find(const T& v) const {
//        const uint32_t total = size();
//#pragma nounroll
//        for (uint32_t i = 0; i < total; ++i) {
//            T* it = (T*) buffer + i;
//            if (v == *it) {
//                return it;
//            }
//        }
//        return nullptr;
//    }
};


template<typename T>
class Array : public ArrayBase {
public:
    using value_type = T;
    static constexpr uint32_t ElementSize = sizeof(T);
    using SizeType = uint32_t;

    Array(std::initializer_list<T> list) noexcept: ArrayBase(nullptr, ArrayTypes<T>::value) {
        ek_core_dbg_inc(EK_CORE_DBG_ARRAY);
        _initCopyMem(list.begin(), (SizeType) list.size());
        // TODO: test length should be valid after initializer_list
    }

    explicit Array(SizeType capacity) noexcept: ArrayBase(nullptr, ArrayTypes<T>::value) {
        ek_core_dbg_inc(EK_CORE_DBG_ARRAY);
        ek_buf_set_capacity(&buffer, capacity, ElementSize);
    }

    /*constexpr*/ Array() noexcept: ArrayBase(nullptr, ArrayTypes<T>::value) {
        ek_core_dbg_inc(EK_CORE_DBG_ARRAY);
    }

    /*constexpr*/ Array(Array&& m) noexcept: ArrayBase(m.buffer, ArrayTypes<T>::value) {
        ek_core_dbg_inc(EK_CORE_DBG_ARRAY);
        m.buffer = nullptr;
    }

    Array(const Array& m) noexcept: ArrayBase(nullptr, ArrayTypes<T>::value) {
        ek_core_dbg_inc(EK_CORE_DBG_ARRAY);
        _initCopyMem((const T*) m.buffer, ek_buf_length(m.buffer));
    }

    Array& operator=(Array&& m) noexcept {
        if (this == &m) {
            return *this;
        }

        if (buffer == m.buffer) {
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

        if (buffer == m.buffer) {
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
            type.construct_copy(begin(), m.begin(), otherSize);
        }
        return *this;
    }

    void push_back(const T& el) {
        maybe_grow();

        auto* buff = (T*) ek_buf_add_(buffer, ElementSize);

        // copy constructor
        new(buff)T(el);
    }

    T& emplace_back(T&& el) {
        maybe_grow();

        // move constructor
        auto* buff = (T*) ek_buf_add_(buffer, ElementSize);
        static_assert(std::is_nothrow_move_constructible<T>::value, "move constructor required");
        new(buff)T(std::move(el));
        //el.~T();
        return *buff;
    }

    template<typename ...Args>
    T& emplace_back(Args&& ...args) {
        maybe_grow();

        auto* buff = (T*) ek_buf_add_(buffer, ElementSize);
        new(buff)T(args...);
        return *buff;
    }

    void eraseIterator(T* el) {
        EK_ASSERT_R2(el != nullptr);
        EK_ASSERT_R2(el >= begin());
        erase_at(el - begin());
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
        auto* p = (T*) ek_buf_pop_(buffer, type.size);
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