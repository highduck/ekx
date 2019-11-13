#pragma once

#include "date.hpp"
#include <fstream>
#include <any>
#include <string>
#include <vector>
#include <map>

// original: https://github.com/animetrics/PlistCpp

// TODO: support old-plain plist format (for example PBXproj)
// - https://github.com/abidon/libpbxparser/blob/master/plist_decoder.cc

namespace ek::plist {

// Plist value types and their corresponding c++ types
using string_type = std::string;
using integer_type = int64_t;
using real_type = double;
using dictionary_type = std::map<std::string, std::any>;
using array_type = std::vector<std::any>;
using date_type = date_t;
using data_type = std::vector<char>;
using boolean_type = bool;

// Public read methods.  Plist type (binary or xml) automatically detected.
void read_plist(const char* byte_array_temp, int64_t size, std::any& message);

void read_plist(std::istream& stream, std::any& message);

template<typename T>
void read_plist(const char* byte_array, int64_t size, T& message);

template<typename T>
void read_plist(std::istream& stream, T& message);

template<typename T>
void read_plist(const char* filename, T& message);

// Public binary write methods.
void write_plist_binary(std::ostream& stream, const std::any& message);

void write_plist_binary(std::vector<char>& plist, const std::any& message);

void write_plist_binary(const char* filename, const std::any& message);

// Public XML write methods.
void write_plist_xml(std::ostream& stream, const std::any& message);

void write_plist_xml(std::vector<char>& plist, const std::any& message);

void write_plist_xml(const char* filename, const std::any& message);

template<typename T>
void read_plist(const char* filename, T& message) {
    std::ifstream stream(filename, std::ios::binary);
    if (!stream) {
        throw std::runtime_error("Can't open file.");
    }
    read_plist(stream, message);
}

template<typename T>
void read_plist(const char* byte_array, int64_t size, T& message) {
    std::any tmp_message;
    read_plist(byte_array, size, tmp_message);
    message = std::any_cast<T>(tmp_message);
}

template<typename T>
void read_plist(std::istream& stream, T& message) {
    std::any tmp_message;
    read_plist(stream, tmp_message);
    message = std::any_cast<T>(tmp_message);
}

}

