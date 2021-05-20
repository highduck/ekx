#pragma once

#include <ek/ds/Array.hpp>
#include <ek/util/Path.hpp>
#include <string>
#include <cstdint>
#include <unordered_map>

struct dirent;

namespace ek {

class output_memory_stream;

void save(const output_memory_stream& stream, const path_t& path);

void save(const output_memory_stream& stream, const std::string& path);

void save(const output_memory_stream& stream, const char *path);

void save(const Array<uint8_t>& buffer, const path_t& path);

void save(const Array<uint8_t>& buffer, const char *path);

void save(const std::string& text, const path_t& path);

Array<uint8_t> read_file(const path_t& path);

bool is_dir(const char *path);

bool is_dir(const path_t& path);

bool is_dir(const std::string& path);

bool is_file(const char *path);

bool is_file(const std::string& path);

bool is_file(const path_t& path);

std::string get_executable_path();

int execute(const std::string& cmd);

std::string read_text(const path_t& path);

void copy_file(const ek::path_t& src, const ek::path_t& dest);

void copy_tree(const ek::path_t& src, const ek::path_t& dest);

bool make_dir(const char *path);

inline bool make_dir(const path_t& path) {
    return make_dir(path.c_str());
}

bool make_dirs(const path_t& path);

inline bool make_dirs(const std::string& path) {
    return make_dirs(path_t{path});
}

Array<path_t> search_files(const std::string& pattern, const path_t& path);

bool remove_dir_rec(const char *path);

inline bool remove_dir_rec(const path_t& path) {
    return remove_dir_rec(path.c_str());
}

void replace_in_file(const path_t& path, const std::unordered_map<std::string, std::string>& substitutions);

#ifndef EK_DISABLE_SYSTEM_FS

bool is_dir_entry_real(const dirent *e);

#endif

}

