#pragma once

#include <vector>
#include <string>

struct ma_resource_manager_data_source;
struct ma_sound;

namespace ek::audio {

void initialize();
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

    ma_sound* getNextSound();
private:
    std::vector<uint8_t> source{};
    ma_resource_manager_data_source* dataSource = nullptr;
    std::string dataSourceFilePath{};
    std::vector<ma_sound*> sounds{};
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

    [[nodiscard]] float getVolume() const;

    void setPitch(float pitch);

    [[nodiscard]] float getPitch() const;

    // poll audio stream update..
    void update();

private:
    float volume_ = 1.0f;
    float pitch_ = 1.0f;
    std::vector<uint8_t> source{};
    std::string dataSourceFilePath{};
    ma_sound* sound = nullptr;
    ma_resource_manager_data_source* dataSource = nullptr;
};

}