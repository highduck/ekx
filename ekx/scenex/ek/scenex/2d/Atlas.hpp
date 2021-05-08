#pragma once

#include "Sprite.hpp"
#include <ek/util/common_macro.hpp>
#include <ek/util/Res.hpp>
#include <cstdint>
#include <string>
#include <ek/ds/Array.hpp>
#include <unordered_map>
#include <functional>

namespace ek {

class Atlas : private disable_copy_assign_t {
public:

    Atlas();

    ~Atlas();

    std::unordered_map<std::string, Res<Sprite>> sprites;
    Array<Res<graphics::Texture>> pages;

    using LoadCallback = std::function<void(Atlas*)>;
    static void load(const char* path, float scaleFactor, const LoadCallback& callback);
};

}

