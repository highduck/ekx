#pragma once

//#include <cstdio>
//#include <string_view>
//#include <string>

namespace ecxx {
namespace details {

//// thank you, stackoverflow.com
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

template<typename Identity>
struct identity_counter {
    static Identity counter;

    inline static Identity next() {
        //printf("ID-next: %d (%s)\n", counter, std::string{type_name<Identity>()}.c_str());
        return counter++;
    }
};

template<typename Identity>
Identity identity_counter<Identity>::counter{0};

}

// 1. match between TU
// 2. starts from 0 for each Identity type
template<typename T, typename Identity>
struct identity_generator {
    static const Identity value;
};

template<typename T, typename Identity>
const Identity identity_generator<T, Identity>::value = details::identity_counter<Identity>::next();

}