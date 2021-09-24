#pragma once

#include "Sprite.hpp"
#include <ek/util/NoCopyAssign.hpp>
#include <ek/util/Res.hpp>
#include <cstdint>
#include <string>
#include <ek/ds/Array.hpp>
#include <unordered_map>
#include <functional>
#include <ek/graphics/TextureLoader.h>

namespace ek {

class Atlas : private NoCopyAssign {
public:

    Atlas();

    ~Atlas();

    void clear();

    std::unordered_map<std::string, Res<Sprite>> sprites;
    Array<Res<graphics::Texture>> pages;
    Array<graphics::TextureLoader*> loaders;

    [[nodiscard]] int getLoadingTexturesCount() const;
    int pollLoading();

    void load(const char* path, float scaleFactor);
};

}

