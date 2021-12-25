#pragma once

#include "auph/auph.h"
#include <ek/util/Type.hpp>

namespace ek {

class AudioResource {
public:
    AudioResource() = default;

    ~AudioResource();

    void load(const char* filepath, bool streaming);

    void unload();

    auph_buffer buffer{};
};

EK_DECLARE_TYPE(AudioResource);

}
