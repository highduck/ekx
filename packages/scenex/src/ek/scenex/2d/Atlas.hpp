#pragma once

#include "Sprite.hpp"
#include <ek/util/NoCopyAssign.hpp>
#include <ek/util/Res.hpp>
#include <ek/util/Type.hpp>
#include <cstdint>
#include <ek/ds/Array.hpp>
#include <functional>
#include <ek/texture_loader.h>

namespace ek {

class Atlas : private NoCopyAssign {
public:

    Atlas();

    ~Atlas();

    void clear();

    Array<Res<Sprite>> sprites;
    Array<Res<graphics::Texture>> pages;
    Array<ek_texture_loader*> loaders;
    uint32_t formatMask = 1;

    [[nodiscard]] int getLoadingTexturesCount() const;
    int pollLoading();

    void load(const char* path, float scaleFactor);
};

EK_DECLARE_TYPE(Atlas);
EK_TYPE_INDEX(Atlas, 8);

}

