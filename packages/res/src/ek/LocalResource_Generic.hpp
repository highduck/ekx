#pragma once

#include "LocalResource.hpp"
#include "LocalResource_System.hpp"

namespace ek {

void get_resource_content_async(const char* path, const get_content_callback_func& callback) {
    callback(internal::read_file_bytes(path));
}

}