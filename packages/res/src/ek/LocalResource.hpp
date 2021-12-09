#pragma once

#include <cstdint>
#include <cstdlib>

#include <functional>

namespace ek {

struct LocalResource;

typedef void (* LocalResourceCloseFn)(LocalResource* lr);

struct LocalResource {
    uint8_t* buffer = nullptr;
    size_t length = 0;
    void* userdata = nullptr;
    int32_t status = 0;
    void* handle = nullptr;
    LocalResourceCloseFn closeFunc = nullptr;

    [[nodiscard]]
    bool success() const {
        return status == 0;
    }

    void close() {
        if(closeFunc) {
            closeFunc(this);
        }
    }
};

#if defined(__EMSCRIPTEN__)
using get_content_callback_func = std::function<void(LocalResource)>;
void get_resource_content_async(const char* path, get_content_callback_func callback);
#else

int getFile_platform(const char* path, LocalResource* lr);

template<typename Fn>
void get_resource_content_async(const char* path, Fn&& callback) {
    LocalResource lr{};
    getFile_platform(path, &lr);
    callback(lr);
}

#endif

}
