#pragma once

#include <ek/util/path.hpp>
#include <ek/ds/Array.hpp>

#include <string>

namespace ek {

std::string current_working_directory();

class working_dir_t {
public:
    working_dir_t();

    explicit working_dir_t(const std::string& new_path);

    explicit working_dir_t(const path_t& new_path);

    ~working_dir_t();

    void push(const std::string& new_path);
    void push(const path_t& new_path);

    std::string pop();

    template<typename Path, typename Func>
    inline void in(const Path& new_path, Func fn) {
        push(new_path);
        fn();
        pop();
    }

    template<typename Func, typename Path>
    inline static void with(const Path& new_path, Func fn) {
        working_dir_t wd{new_path};
        fn();
    }

    [[nodiscard]] const std::string& current() const {
        return st_.back();
    }

private:
    Array<std::string> st_;
};

}

