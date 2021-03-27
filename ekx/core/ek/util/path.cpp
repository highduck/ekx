#include "path.hpp"
#include <sstream>

namespace ek {

std::ostringstream& operator<<(std::ostringstream& ss, const path_t& path) noexcept {
    ss << path.str();
    return ss;
}

std::string path_join(const std::string& p1, const std::string& p2) {
    if (p1.empty()) {
        return p2;
    }

    if (p2.empty()) {
        return p1;
    }

    const char sep = '/';
    if (p1[p1.size() - 1] != sep) {
        return p1 + sep + p2;
    }
    return p1 + p2;
}

std::string path_ext(const std::string& path) {
    using std::string;
    const auto i = path.find_last_of('.');
    if (i != string::npos) {
        return path.substr(i);
    }
    return "";
}

std::string path_without_ext(const std::string& path) {
    const auto i = path.find_last_of('.');
    if (i != std::string::npos) {
        return path.substr(0, i);
    }
    return path;
}

std::string path_dir(const std::string& path) {
    const auto i = path.find_last_of('/');
    if (i != std::string::npos) {
        return path.substr(0, i);
    }
    return "";
}

std::string path_name(const std::string& path) {
    using std::string;
    const auto i = path.find_last_of('/');
    if (i != string::npos) {
        return path.substr(i + 1);
    }
    return path;
}

}