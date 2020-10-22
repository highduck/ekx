#pragma once

#include <cassert>

namespace ek {

template<typename T>
struct assert_single_instance_t {
    static int count;

    assert_single_instance_t() {
        assert(assert_single_instance_t<T>::count == 0);
        ++assert_single_instance_t<T>::count;
    }

    ~assert_single_instance_t() {
        --assert_single_instance_t<T>::count;
        assert(assert_single_instance_t<T>::count == 0);
    }
};

template<typename T>
inline int assert_single_instance_t<T>::count{0};

template<typename T>
struct assert_created_once_t {
    static char count;
};

template<typename T>
char assert_created_once_t<T>::count = 0;

template<typename T>
inline void assert_created_once() {
    const auto current_count = assert_created_once_t<T>::count;
    assert(current_count == 0);
    assert_created_once_t<T>::count = current_count + 1;
}

struct disable_copy_assign_t {
    disable_copy_assign_t() = default;

    ~disable_copy_assign_t() = default;

    disable_copy_assign_t(const disable_copy_assign_t& v) = delete;

    disable_copy_assign_t& operator=(const disable_copy_assign_t& v) = delete;

    disable_copy_assign_t(disable_copy_assign_t&& v) = delete;

    disable_copy_assign_t& operator=(disable_copy_assign_t&& v) = delete;
};

}