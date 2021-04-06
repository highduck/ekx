#include "system.hpp"

#include <ek/util/logger.hpp>
#include <ek/serialize/streams.hpp>
#include <ek/util/strings.hpp>

#include <cstdio>
#include <fstream>
#include <array>
#include <vector>

#if defined(__ANDROID__)

#define EK_DISABLE_SYSTEM_FS

#endif

#ifndef EK_DISABLE_SYSTEM_FS

#include <ftw.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>

#endif

#if defined(__APPLE__)

#include <mach-o/dyld.h>

#endif

namespace ek {

const int system_pathMaxSize = 4096;

void save(const output_memory_stream& stream, const path_t& path) {
    save(stream, path.c_str());
}

void save(const output_memory_stream& stream, const std::string& path) {
    save(stream, path.c_str());
}

void save(const output_memory_stream& stream, const char* path) {
    auto h = fopen(path, "wb");
    if (h) {
        fwrite(stream.data(), 1, stream.size(), h);
        fclose(h);
    } else {
        EK_WARN << "fopen error: " << path;
    }
}

void save(const std::vector<uint8_t>& buffer, const path_t& path) {
    save(buffer, path.c_str());
}

void save(const std::vector<uint8_t>& buffer, const char* path) {
    auto h = fopen(path, "wb");
    if (h) {
        fwrite(buffer.data(), 1, buffer.size(), h);
        fclose(h);
    } else {
        EK_WARN << "fopen error: " << path;
    }
}

void save(const std::string& text, const path_t& path) {
    std::ofstream s{path.c_str()};
    if (s.is_open()) {
        s << text;
        s.flush();
    } else {
        EK_WARN << "fstream open error: " << path;
    }
}

/** check path for system FS **/

bool is_dir(const char* path) {
#ifndef EK_DISABLE_SYSTEM_FS
    struct stat sb{};
    return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
#else
    return false;
#endif
}

bool is_dir(const path_t& path) {
    return is_dir(path.c_str());
}

bool is_dir(const std::string& path) {
    return is_dir(path.c_str());
}

bool is_file(const char* path) {
#ifndef EK_DISABLE_SYSTEM_FS
    struct stat sb{};
    return stat(path, &sb) == 0 && S_ISREG(sb.st_mode);
#else
    return false;
#endif
}

bool is_file(const std::string& path) {
    return is_file(path.c_str());
}

bool is_file(const path_t& path) {
    return is_file(path.c_str());
}

std::string get_executable_path() {
#if defined(_WIN32)
    // TODO:
#elif defined(__APPLE__)
    char path[system_pathMaxSize + 1];
    uint32_t path_len = system_pathMaxSize;
    if (_NSGetExecutablePath(path, &path_len)) {
        return {};
    }
    return path;
#elif defined(__ANDROID__)
    return "";
#elif defined(__linux__)
    char path[system_pathMaxSize];
    const auto length = readlink("/proc/self/exe", path, system_pathMaxSize);
    return length < 0 ? "" : std::string{path, static_cast<size_t>(length)};
#endif
}

int execute(const std::string& cmd) {
#ifndef EK_DISABLE_SYSTEM_FS
//    return system(cmd.c_str());
    std::array<char, 128> buffer{};
//    std::string result;

    auto pipe = popen(cmd.c_str(), "r");

    if (!pipe) {
//        throw std::runtime_error("popen() failed!");

        return -1;
    }

    while (!feof(pipe)) {
        auto* ret = fgets(buffer.data(), buffer.size(), pipe);
        if (ret != nullptr) {
            puts(buffer.data());
        }
    }

    return pclose(pipe);
#else
    return -1;
#endif
}

// continue utilities

void copy_file(const path_t& src, const path_t& dest) {
    std::ifstream stream_src(src.str(), std::ios::binary);
    std::ofstream stream_dest(dest.str(), std::ios::binary);
    stream_dest << stream_src.rdbuf();
}

#ifndef EK_DISABLE_SYSTEM_FS

bool is_dir_entry_real(const dirent* e) {
    // empty
    if (!e || e->d_name[0] == 0) {
        return false;
    }
    if (e->d_name[0] == '.') {
        // "."
        if (e->d_name[1] == 0) {
            return false;
        }
        // ".."
        if (e->d_name[1] == '.' && e->d_name[2] == 0) {
            return false;
        }
    }
    return true;
}

#endif

void copy_tree(const ek::path_t& src, const ek::path_t& dest) {
    if (!is_dir(src)) {
        EK_WARN << "IS NOT DIR: " << src.str();
        return;
    }
#ifndef EK_DISABLE_SYSTEM_FS

    DIR* dir = opendir(src.c_str());
    if (dir) {
        if (!is_dir(dest)) {
            EK_INFO("MAKE DIR: %s \n", dest.c_str());
            make_dirs(dest);
        }

        std::vector<std::string> nested_dirs;
        struct dirent* e;
        while ((e = readdir(dir)) != nullptr) {
            if (is_dir_entry_real(e)) {
                if ((e->d_type & DT_DIR) != 0) {
                    nested_dirs.emplace_back(e->d_name);
                } else if ((e->d_type & DT_REG) != 0) {
                    const auto c_src = src / e->d_name;
                    const auto c_dst = dest / e->d_name;
                    EK_TRACE("COPY FILE: %s -> %s", c_src.c_str(), c_dst.c_str());
                    if (is_file(c_src)) {
                        copy_file(c_src, c_dst);
                    } else {
                        EK_WARN("NO FILE: %s", c_src.c_str());
                    }
                }
            }
        }
        closedir(dir);

        for (const auto& nested_dir : nested_dirs) {
            copy_tree(src / nested_dir, dest / nested_dir);
        }
    }
#endif
}

bool make_dir(const char* path) {
#ifndef EK_DISABLE_SYSTEM_FS
    int err = 0;
#if defined(_WIN32)
    err = _mkdir(path); // can be used on Windows
#else
    mode_t mode = 0733; // UNIX style permissions
    err = mkdir(path, mode); // can be used on non-Windows
#endif
    return err == 0;
#else
    return false;
#endif
}

bool make_dirs(const path_t& path) {
#ifndef EK_DISABLE_SYSTEM_FS
    if (is_dir(path)) {
        return true;
    }

    std::vector<std::string> list = split(path.str(), '/');
    std::string pit{};
    for (const auto& part : list) {
        if (!part.empty()) {
            pit += part + '/';
            if (is_file(pit.c_str())) {
                EK_ERROR << "can't create dir because already file: " << pit;
                return false;
            }
            if (!is_dir(pit.c_str())) {
                make_dir(pit.c_str());
            }
        }
    }
#endif
    return true;
}


std::string read_text(const path_t& path) {
    std::ifstream s{path.c_str()};
    return std::string{
            std::istreambuf_iterator<char>(s),
            std::istreambuf_iterator<char>()
    };
}

#ifndef EK_DISABLE_SYSTEM_FS

static int remove_dir_rec_cb(const char* path, const struct stat*, int, struct FTW*) {
    auto ret = remove(path);
    if (ret != 0) {
        EK_ERROR("ERROR rmdir_cb : %i", ret);
    }
    return ret;
}

#endif

bool remove_dir_rec(const char* path) {
#ifndef EK_DISABLE_SYSTEM_FS
    if (is_dir(path)) {
        return nftw(path, remove_dir_rec_cb, 20, FTW_DEPTH) == 0;
    }
#endif
    return false;
}

void replace_in_file(const path_t& path, const std::unordered_map<std::string, std::string>& substitutions) {
    auto res = read_text(path);
    for (const auto& kv : substitutions) {
        res = ek::replace(res, kv.first, kv.second);
    }
    save(res, path);
}

void search_files(const std::string& pattern, const path_t& path, std::vector<path_t>& out) {
    using std::vector;
    using std::string;
#ifndef EK_DISABLE_SYSTEM_FS
    const char* path_dir = path.empty() ? "." : path.c_str();
    DIR* dir = opendir(path_dir);
    if (dir) {
        vector<string> nested_dirs;
        struct dirent* e;
        while ((e = readdir(dir)) != nullptr) {
            if (is_dir_entry_real(e)) {
                path_t fullname = path / e->d_name;
                if (fnmatch(pattern.c_str(), fullname.c_str(), 0) == 0) {
                    out.emplace_back(fullname);
                }
                if ((e->d_type & DT_DIR) != 0) {
                    nested_dirs.emplace_back(e->d_name);
                }
            }
        }
        closedir(dir);

        for (const auto& nested_dir : nested_dirs) {
            search_files(pattern, path / nested_dir, out);
        }
    }
#endif
}

std::vector<path_t> search_files(const std::string& pattern, const path_t& path) {
    using std::vector;
    using std::string;

    vector<path_t> res;
#ifndef EK_DISABLE_SYSTEM_FS
    const char* path_dir = path.empty() ? "." : path.c_str();
    if (!is_dir(path_dir)) {
        EK_WARN << "IS NOT DIR: " << path_dir;
        return res;
    }
    search_files(pattern, path, res);
#endif
    return res;
}

std::vector<uint8_t> read_file(const path_t& path) {
    std::vector<uint8_t> buffer;
    auto* stream = fopen(path.c_str(), "rb");
    if (stream) {
        fseek(stream, 0, SEEK_END);
        buffer.resize(static_cast<size_t>(ftell(stream)));
        fseek(stream, 0, SEEK_SET);

        fread(buffer.data(), buffer.size(), 1u, stream);

        if (ferror(stream) != 0) {
            buffer.resize(0);
            buffer.shrink_to_fit();
        }

        fclose(stream);
    }
    return buffer;
}

}