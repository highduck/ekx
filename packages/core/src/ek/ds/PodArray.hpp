#pragma once

#include <ek/buf.h>
#include <ek/assert.h>
#include <ek/core_dbg.h>

// for Array
#include <initializer_list>
#include <utility>

namespace ek {

template<typename T>
class PodArray final {
public:
    using value_type = T;
    T* buffer;

    PodArray(std::initializer_list<T> list) noexcept: buffer{nullptr} {
        ek_core_dbg_inc(EK_CORE_DBG_POD_ARRAY);
        arr_init_from((void**) &buffer, sizeof(T), list.begin(), (uint32_t) list.size());
        // TODO: test length should be valid after initializer_list
    }

    explicit PodArray(uint32_t capacity) noexcept: buffer{nullptr} {
        ek_core_dbg_inc(EK_CORE_DBG_POD_ARRAY);
        ek_buf_set_capacity((void**) &buffer, capacity, sizeof(T));
    }

    /*constexpr*/ PodArray() noexcept: buffer{nullptr} {
        ek_core_dbg_inc(EK_CORE_DBG_POD_ARRAY);
    }

    /*constexpr*/ PodArray(PodArray&& m) noexcept: buffer{m.buffer} {
        ek_core_dbg_inc(EK_CORE_DBG_POD_ARRAY);
        m.buffer = nullptr;
    }

    PodArray(const PodArray& m) noexcept: buffer{nullptr} {
        arr_init_from((void**) &buffer, sizeof(T), m.buffer, ek_buf_length(m.buffer));
    }

    ~PodArray() {
        ek_core_dbg_dec(EK_CORE_DBG_POD_ARRAY);
        reset();
    }

    void reset() {
        ek_buf_reset((void**) &buffer);
    }

    [[nodiscard]]
    bool empty() const {
        return ek_buf_empty(buffer);
    }

    void clear() const {
        if (buffer) {
            ek_buf_header(buffer)->length = 0;
        }
    }

    [[nodiscard]]
    uint32_t size() const {
        return ek_buf_length(buffer);
    }

    [[nodiscard]]
    uint32_t capacity() const {
        return ek_buf_capacity(buffer);
    }

    PodArray& operator=(PodArray&& m) noexcept {
        if (this == &m || buffer == m.buffer) {
            return *this;
        }

        // destruct all prev allocated elements
        // and free previous storage if allocated
        reset();

        buffer = m.buffer;
        m.buffer = nullptr;
        return *this;
    }

    PodArray& operator=(const PodArray& m) noexcept {
        // instances are literally the same,
        // also return on no-op empty to empty case

        // TODO: add test for assigning self to self
        if (this == &m || buffer == m.buffer) {
            return *this;
        }

        arr_assign((void**) &buffer, sizeof(T), m.buffer);
        return *this;
    }

//    ~PodArray() noexcept {
//    }

    void reserve(uint32_t capacity) {
        if (ek_buf_capacity(buffer) < capacity) {
            arr_grow((void**) &buffer, capacity, sizeof(T));
        }
    }

    void push_back(const T& el) {
        arr_push_mem((void**) &buffer, sizeof(T), &el);
    }

    T& emplace_back(T&& el) {
        return *(T*) arr_push_mem((void**) &buffer, sizeof(T), &el);
    }

    template<typename ...Args>
    T& emplace_back(Args&& ...args) {
        T el(args...);
        return *(T*) arr_push_mem((void**) &buffer, sizeof(T), &el);
    }

    void resize(uint32_t new_size) {
        arr_resize((void**) &buffer, sizeof(T), new_size);
    }

    void erase_ptr(T* el) {
        EK_ASSERT_R2(buffer);
        EK_ASSERT_R2(el);
        EK_ASSERT_R2(el >= buffer);
        erase_at(el - buffer);
    }

    void erase_at(uint32_t i) {
        arr_remove(buffer, sizeof(T), i);
    }

    void set(uint32_t i, const T& el) {
        EK_ASSERT_R2(i < size());
        buffer[i] = el;
    }

    [[nodiscard]]
    T& get(uint32_t i) const {
        EK_ASSERT_R2(i < size());
        return buffer[i];
    }

    void pop_back() {
        arr_pop(buffer);
    }

    bool remove(const T& el) {
#pragma nounroll
        for (uint32_t i = 0; i < size(); ++i) {
            if (el == buffer[i]) {
                erase_at(i);
                return true;
            }
        }
        return false;
    }

    void swap_remove(uint32_t i) {
        arr_swap_remove(buffer, sizeof(T), i);
    }

    [[nodiscard]]
    T& back() const {
        EK_ASSERT(!empty());
        return buffer[size() - 1];
    }

    [[nodiscard]]
    T& front() const {
        EK_ASSERT(!empty());
        return buffer[0];
    }

    T* begin() const {
        return buffer;
    }

    T* end() const {
        return buffer + size();
    }

    T& operator[](uint32_t i) const {
        EK_ASSERT_R2(i < size());
        return buffer[i];
    }

    [[nodiscard]]
    T* data() const {
        return buffer;
    }

    T* find(const T& v) const {
        return (T*) arr_search(buffer, sizeof(T), &v);
    }
};

}