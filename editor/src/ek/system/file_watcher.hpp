#pragma once

#include <ek/fs/path.hpp>
#include <unordered_map>
#include <string>

namespace ek {

class file_watcher_t {
public:
    explicit file_watcher_t(const path_t& path);

    ~file_watcher_t();

    void update(const std::function<void(path_t)>& callback);

private:
    std::function<void(path_t)> callback_;
    std::unordered_map<std::string, uint64_t> list_;
    path_t path_;
};

}

