#pragma once

#include <vector>
#include <string>
#include <auph/auph.hpp>

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
