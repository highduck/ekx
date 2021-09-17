#pragma once

#include <string>
#include <utility>

namespace ek {

std::string path_join(const std::string& p1, const std::string& p2);

std::string path_ext(const std::string& path);

std::string path_without_ext(const std::string& path);

std::string path_dir(const std::string& path);

std::string path_name(const std::string& path);

class path_t {
public:
    path_t() noexcept = default;

    explicit path_t(const char* c_str) noexcept : path_{c_str} {
    }

    explicit path_t(std::string str) noexcept : path_{std::move(str)} {
    }

    path_t operator+(const std::string& str) const noexcept {
        return path_t{path_ + str};
    }

    path_t operator+(const char* str) const noexcept {
        return path_t{path_ + str};
    }

    path_t operator/(const path_t& path) const noexcept {
        return path_t{path_join(path_, path.path_)};
    }

    path_t operator/(const char* path) const noexcept {
        return path_t{path_join(path_, std::string{path})};
    }

    friend path_t operator/(const char* path1, const path_t& path2) noexcept {
        return path_t{path_join({path1}, path2.str())};
    }

    friend path_t operator/(const std::string& path1, const path_t& path2) noexcept {
        return path_t{path_join(path1, path2.str())};
    }

    path_t operator/(const std::string& path) const noexcept {
        return path_t{path_join(path_, path)};
    }

    [[nodiscard]] std::string withoutExt() const {
        return path_without_ext(path_);
    }

    [[nodiscard]] std::string ext() const {
        return path_ext(path_);
    }

    [[nodiscard]] path_t dir() const {
        return path_t{path_dir(path_)};
    }

    [[nodiscard]] path_t basename() const {
        return path_t{path_name(path_)};
    }

    [[nodiscard]] const std::string& str() const {
        return path_;
    }

    [[nodiscard]] const char* c_str() const {
        return path_.c_str();
    }

    [[nodiscard]] bool empty() const {
        return path_.empty();
    }


private:
    std::string path_;
};

}


