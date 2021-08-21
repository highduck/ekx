#pragma once

#include <vector>
#include <string>
#include <auph/auph.hpp>

namespace ek::audio {

void initialize();
void start();
void shutdown();

void muteDeviceBegin();

void muteDeviceEnd();

class Sound {
public:
    Sound() = default;

    explicit Sound(const char* path);

    ~Sound();

    void load(const char* path);

    void unload();

    void play(float volume = 1.0f, float pitch = 1.0f);

    auph::Buffer buffer{};
private:
    std::string dataSourceFilePath{};
};

class Music {
public:
    Music() = default;

    explicit Music(const char* path);

    ~Music();

    void load(const char* path);

    void unload();

    void play(float volume, float pitch);

    void stop();

    void setVolume(float volume);

    [[nodiscard]] float getVolume() const;

    void setPitch(float pitch);

    [[nodiscard]] float getPitch() const;

    auph::Buffer buffer{};
    auph::Voice voice{};

private:
    std::string dataSourceFilePath{};
};

}
