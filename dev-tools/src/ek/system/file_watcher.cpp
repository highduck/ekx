#include "file_watcher.hpp"
#include "system.hpp"

namespace ek {
//
//void scan_path(const path_t& path, std::unordered_map<path_t, uint64_t>& file_map) {
//    DIR* dir = opendir(path.c_str());
//    if (dir) {
//        struct dirent* e;
//        while ((e = readdir(dir)) != nullptr) {
//            if (is_dir_entry_real(e)) {
//                if ((e->d_type & DT_DIR) != 0) {
//                    nested_dirs.emplace_back(e->d_name);
//                } else if ((e->d_type & DT_REG) != 0) {
//                    const auto c_src = src / e->d_name;
//                    const auto c_dst = dest / e->d_name;
//                    EK_TRACE("COPY FILE: %s -> %s", c_src.c_str(), c_dst.c_str());
//                    if (is_file(c_src)) {
//                        copy_file(c_src, c_dst);
//                    } else {
//                        EK_WARN("NO FILE: %s", c_src.c_str());
//                    }
//                }
//            }
//        }
//        closedir(dir);
//
//        for (const auto& nested_dir : nested_dirs) {
//            copy_tree(src / nested_dir, dest / nested_dir);
//        }
//    }
//}

file_watcher_t::file_watcher_t(const path_t& path) {

}

file_watcher_t::~file_watcher_t() = default;

void file_watcher_t::update(const std::function<void(path_t)>& callback) {

}

}