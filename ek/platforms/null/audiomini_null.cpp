#include <ek/audiomini.hpp>

namespace ek {

AudioMini::AudioMini() = default;

AudioMini::~AudioMini() = default;

void AudioMini::create_sound(const char* name) {}

void AudioMini::create_music(const char* name) {}

void AudioMini::play_sound(const char* name, float vol, float pan) {}

void AudioMini::play_music(const char* name, float vol) {}

void AudioMini::vibrate(int duration_millis) {}

}