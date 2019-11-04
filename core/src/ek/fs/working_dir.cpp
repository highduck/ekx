#include <zconf.h>
#include "working_dir.hpp"
#include "system.hpp"

namespace ek {

/** Working directory utility **/
const int path_max_size = 4096;

std::string current_working_directory() {
    char buf[path_max_size];
    getcwd(buf, path_max_size);
    return std::string{buf};
}

working_dir_t::working_dir_t() {
    st_.emplace_back(current_working_directory());
}

working_dir_t::working_dir_t(const std::string& new_path)
        : working_dir_t() {
    push(new_path);
}

working_dir_t::working_dir_t(const path_t& new_path)
        : working_dir_t(new_path.str()) {
}

working_dir_t::~working_dir_t() {
    if (st_.size() > 1) {
        chdir(st_.front().c_str());
    }
}

void working_dir_t::push(const std::string& new_path) {
    st_.emplace_back(new_path);
    chdir(new_path.c_str());
}

std::string working_dir_t::pop() {
    auto back = st_.back();
    st_.pop_back();
    return back;
}

}