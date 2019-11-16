#include <ek/audiomini.hpp>

extern "C" {

extern void audio_mini_vibrate(int millis);
extern void audio_mini_play_sound(const char* name, float vol, float pan);
extern void audio_mini_play_music(const char* name, float vol);
extern void audio_mini_create_sound(const char* name);
extern void audio_mini_create_music(const char* name);

}

namespace ek {

AudioMini::AudioMini() = default;

AudioMini::~AudioMini() = default;

void AudioMini::create_sound(const char* name) {
    audio_mini_create_sound(name);
}

void AudioMini::create_music(const char* name) {
    audio_mini_create_music(name);
}

void AudioMini::play_sound(const char* name, float vol, float pan) {
    audio_mini_play_sound(name, vol, pan);
}

void AudioMini::play_music(const char* name, float vol) {
    audio_mini_play_music(name, vol);
}

void AudioMini::vibrate(int millis) {
    audio_mini_vibrate(millis);
}

}