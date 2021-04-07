#pragma once

#include <ek/app/res.hpp>
#include <ek/app/internal/res_sys.hpp>

namespace ek {

void get_resource_content_async(const char* path, const get_content_callback_func& callback) {
    callback(internal::read_file_bytes(path));
}

}