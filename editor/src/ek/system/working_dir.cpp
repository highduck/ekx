#include "working_dir.hpp"
#include "system.hpp"

#include <unistd.h>
#include <ek/util/logger.hpp>

namespace ek {

/** Working directory util **/
const int path_max_size = 4096;

std::string current_working_directory() {
    char buf[path_max_size];
    ::getcwd(buf, path_max_size);
    return std::string{buf};
}

working_dir_t::working_dir_t() {
    st_.emplace_back(current_working_directory());
    //EK_DEBUG << "BEGIN working dir: " << st_.front();
}

working_dir_t::working_dir_t(const std::string& new_path)
        : working_dir_t() {
    push(new_path);
}

working_dir_t::working_dir_t(const path_t& new_path)
        : working_dir_t(new_path.str()) {
}

working_dir_t::~working_dir_t() {
    if (!st_.empty()) {
        ::chdir(st_.front().c_str());
//        EK_DEBUG << "END working dir: " << st_.front();
    } else {
//        EK_ERROR << "END working dir EMPTY!";
    }
}

void working_dir_t::push(const std::string& new_path) {
    if (!is_dir(new_path)) {
        EK_ERROR << "[working-dir] cannot change cwd to non-existed directory: " << new_path;
    }

    char resolved_path[PATH_MAX];
    realpath(new_path.c_str(), resolved_path);

    if (!is_dir(resolved_path)) {
        EK_ERROR << "[working-dir] resolved path is not directory: " << resolved_path;
    }

    st_.emplace_back(resolved_path);
    ::chdir(resolved_path);
//    EK_DEBUG << "   PUSH working dir: " << new_path;
}

void working_dir_t::push(const path_t& new_path) {
    push(new_path.str());
}

std::string working_dir_t::pop() {
    auto back = st_.back();
    st_.pop_back();
    if (!st_.empty()) {
        ::chdir(st_.back().c_str());
//        EK_DEBUG << "   POP working dir to " << st_.back();
    }
    return back;
}

}