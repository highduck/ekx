#pragma once

#include <pugixml.hpp>
#include <vector>
#include <string>

#include <cstdio>
#define SPRITE_PACKER_LOG(x, ...) printf(x, __VA_ARGS__)

#include <cassert>
#define SPRITE_PACKER_ASSERT(x) assert(x)

namespace sprite_packer {

struct RectI {
    int x;
    int y;
    int w;
    int h;
};

struct Rect {
    float x;
    float y;
    float w;
    float h;
};

}