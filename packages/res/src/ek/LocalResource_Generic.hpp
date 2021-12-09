#pragma once

#include "LocalResource.hpp"
#include "LocalResource_System.hpp"

namespace ek {

int getFile_platform(const char* path, LocalResource* lr) {
    return getFile(path, lr);
}

}