#pragma once

#include <ek/local_res.h>
#include <functional>

namespace ek {

#if defined(__EMSCRIPTEN__)

using get_content_callback_func = std::function<void(ek_local_res)>;
void get_resource_content_async(const char* path, get_content_callback_func callback);

#else

template<typename Fn>
inline void get_resource_content_async(const char* path, Fn&& callback) {
    ek_local_res lr{};
    ek_local_res_get_file_platform(path, &lr);
    callback(lr);
}

#endif

}