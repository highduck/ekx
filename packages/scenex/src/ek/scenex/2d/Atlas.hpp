#pragma once

#include "Sprite.hpp"
#include <ek/util/NoCopyAssign.hpp>
#include <ek/util/Res.hpp>
#include <ek/util/Type.hpp>
#include <cstdint>
#include <ek/ds/String.hpp>
#include <ek/ds/Array.hpp>
#include <functional>
#include <ek/texture_loader.h>

namespace ek {

class Atlas : private NoCopyAssign {
public:

    Atlas();

    ~Atlas();

    void clear();

    Array<REF_TO(Sprite)> sprites;
    Array<REF_TO(sg_image)> pages;

    // loading routine
    Array<ek_texture_loader*> loaders;
    String base_path;
    uint32_t formatMask = 1;

    [[nodiscard]] int getLoadingImagesCount() const;
    int pollLoading();

    void load(const char* path, float scaleFactor);
};

EK_DECLARE_TYPE(Atlas);
EK_TYPE_INDEX(Atlas, 8);

}

