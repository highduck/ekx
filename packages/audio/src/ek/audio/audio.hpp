#pragma once

#include <auph/auph.hpp>
#include <ek/util/Type.hpp>

namespace ek::audio {

void initialize();

void shutdown();

void muteDeviceBegin();

void muteDeviceEnd();

class AudioResource {
public:
    AudioResource() = default;

    ~AudioResource();

    void load(const char* filepath, bool streaming);

    void unload();

    auph::Buffer buffer{};
};

}

namespace ek {

EK_DECLARE_TYPE(audio::AudioResource);
EK_TYPE_INDEX(audio::AudioResource, 7);

}
