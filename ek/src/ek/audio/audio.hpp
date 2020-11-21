#pragma once

#include <vector>

struct Sound;
struct Music;

namespace ek::audio {

void init();
void muteDeviceBegin();
void muteDeviceEnd();

class Sound {
public:
    Sound() = default;
    explicit Sound(const char* path);
    ~Sound();
    void load(const char* path);
    void unload();
    void play(float volume = 1.0f, bool multi = false);
private:
    ::Sound* ptrHandle = nullptr;
};

class Music {
public:
    Music() = default;
    explicit Music(const char* path);
    ~Music();
    void load(const char* path);
    void unload();
    void play();
    void stop();
    void setVolume(float volume);
    [[nodiscard]]
    float getVolume() const;

    // poll audio stream update..
    void update();
private:
    float volume_{1.0f};
    std::vector<uint8_t> source{};
    ::Music* ptrHandle = nullptr;
};

}
