#pragma once

namespace ek {

struct disable_copy_assign_t {
    disable_copy_assign_t() = default;

    ~disable_copy_assign_t() = default;

    disable_copy_assign_t(const disable_copy_assign_t& v) = delete;

    disable_copy_assign_t& operator=(const disable_copy_assign_t& v) = delete;

    disable_copy_assign_t(disable_copy_assign_t&& v) = delete;

    disable_copy_assign_t& operator=(disable_copy_assign_t&& v) = delete;
};

}