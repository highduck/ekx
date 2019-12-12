#pragma once

#include <cstdint>

//#include <string_view>

// thank you, stackoverflow.com
//template<typename T>
//constexpr auto type_name() {
//    std::string_view name, prefix, suffix;
//#ifdef __clang__
//    name = __PRETTY_FUNCTION__;
//    prefix = "auto type_name() [T = ";
//    suffix = "]";
//#elif defined(__GNUC__)
//    name = __PRETTY_FUNCTION__;
//    prefix = "constexpr auto type_name() [with T = ";
//    suffix = "]";
//#elif defined(_MSC_VER)
//    name = __FUNCSIG__;
//    prefix = "auto __cdecl type_name<";
//    suffix = ">(void)";
//#endif
//    name.remove_prefix(prefix.size());
//    name.remove_suffix(suffix.size());
//    return name;
//}

namespace ek {
namespace details {

template<typename Kind>
struct type_index_generator {
    static uint32_t counter;

    inline static uint32_t next() {
        return counter++;
    }
};

template<typename Kind>
uint32_t type_index_generator<Kind>::counter{0};

}

// 1. match between TU
// 2. starts from 0 for each Kind types family
template<typename T, typename Kind = void>
struct type_index {
    static const uint32_t value;
};

template<typename T, typename Kind>
const uint32_t type_index<T, Kind>::value = details::type_index_generator<Kind>::next();

}