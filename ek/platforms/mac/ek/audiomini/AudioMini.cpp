#include <ek/audiomini/AudioMini.h>
#include "SimpleAudioEngine.h"

namespace ek {

using CocosDenshion::SimpleAudioEngine;

AudioMini::AudioMini() = default;

AudioMini::~AudioMini() = default;

void AudioMini::create_sound(const char* name) {
    if (name == nullptr) return;
    auto* audio = SimpleAudioEngine::getInstance();
    audio->preloadEffect(name);
}

void AudioMini::create_music(const char* name) {
    if (name == nullptr) return;
    auto* audio = SimpleAudioEngine::getInstance();
    audio->preloadBackgroundMusic(name);
}

void AudioMini::play_sound(const char* name, float vol, float pan) {
    if (name == nullptr) return;
    SimpleAudioEngine::getInstance()->playEffect(name, false, 1.0, pan, vol);
}

void AudioMini::play_music(const char* name, float vol) {
    if (name == nullptr) return;
    auto* audio = SimpleAudioEngine::getInstance();
    if (!audio->isBackgroundMusicPlaying()) {
        audio->playBackgroundMusic(name, true);
    }
    audio->setBackgroundMusicVolume(vol);
}

void AudioMini::vibrate(int duration_millis) {
    if (duration_millis <= 0) return;
//        AudioServicesPlayAlertSound(kSystemSoundID_Vibrate);
}

}