#pragma once

#include <string>
#include <utility>

namespace ek {

//void path_append(const char* p1, const char* p2);

inline bool isPathDelimiter(char ch) {
    return ch == '/' || ch == '\\';
}

/**
 * Find path extension
 * @param path - null-terminated string with file path
 * @return pointer to extension in original `path` null-terminated string
 *  - `ext` for `a/b/c.d.e.ext` (string view points to the first extension character)
 *  - `` for `a/b/c/d` (string view points to the end of original string)
 *  - nullptr if `path` is nullptr
 */
const char* path_ext(const char* path);

/**
 * Find name from file path
 * @param path - null-terminated string with file path
 * @return pointer to `name` in original `path` null-terminated string
 *  - `c.d.e.ext` for `a/b/c.d.e.ext` (string view points to the first extension character)
 *  - `` for `a/b/c/` (string view points to the end of original string)
 *  - nullptr if `path` is nullptr
 */
const char* path_name(const char* path);

void path_extract_dir(char* dst, size_t maxSize, const char* path);

class path_t {
public:
    path_t() noexcept = default;

    explicit path_t(const char* c_str) noexcept: path_{c_str} {
    }

    explicit path_t(std::string str) noexcept: path_{std::move(str)} {
    }

    path_t operator+(const std::string& str) const noexcept {
        return path_t{path_ + str};
    }

    path_t operator+(const char* str) const noexcept {
        return path_t{path_ + str};
    }

    path_t operator/(const path_t& path) const noexcept {
        path_t r{path_};
        append(r.path_, path.c_str());
        return r;
    }

    path_t operator/(const char* path) const noexcept {
        path_t r{path_};
        append(r.path_, path);
        return r;
    }

    friend path_t operator/(const char* path1, const path_t& path2) noexcept {
        path_t r{path1};
        append(r.path_, path2.c_str());
        return r;
    }

    friend path_t operator/(const std::string& path1, const path_t& path2) noexcept {
        path_t r{path1};
        append(r.path_, path2.c_str());
        return r;
    }

    path_t operator/(const std::string& path) const noexcept {
        path_t r{path_};
        append(r.path_, path.c_str());
        return r;
    }

    [[nodiscard]] std::string withoutExt() const {
        const char* ext = path_ext(path_.c_str());
        if (ext && *ext != '\0') {
            return {path_.c_str(), ext - 1};
        }
        return {};
    }

    [[nodiscard]] std::string ext() const {
        return path_ext(path_.c_str());
    }

    [[nodiscard]] path_t dir() const {
        char buf[1024];
        path_extract_dir(buf, 1024, path_.c_str());
        return path_t{buf};
    }

    [[nodiscard]] path_t basename() const {
        return path_t{path_name(path_.c_str())};
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

    static void append(std::string& dest, const char* path) {
        if (path != nullptr) {
            // skip all DELIMITER characters in start of `path`
            while (isPathDelimiter(*path)) {
                ++path;
            }
            if (!dest.empty()) {
                const auto lastChar = dest.back();
                if (!isPathDelimiter(lastChar)) {
                    dest.push_back('/');
                }
            }
            dest += path;
        }
    }

private:
    std::string path_;
};

}


