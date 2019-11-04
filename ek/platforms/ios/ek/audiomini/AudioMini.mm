#include <ek/audiomini/AudioMini.h>
#include "SimpleAudioEngine_objc.h"

namespace ek {

AudioMini::AudioMini() = default;

AudioMini::~AudioMini() = default;

void AudioMini::create_sound(const char* name) {
    if (name == nullptr) return;
    SimpleAudioEngine* audio = [SimpleAudioEngine sharedEngine];
    NSString* ns_name = [NSString stringWithUTF8String: name];
    [audio preloadEffect: ns_name];
}

void AudioMini::create_music(const char* name) {
    if (name == nullptr) return;
    SimpleAudioEngine* audio = [SimpleAudioEngine sharedEngine];
    NSString* ns_name = [NSString stringWithUTF8String: name];
    [audio preloadBackgroundMusic: ns_name];
}

void AudioMini::play_sound(const char* name, float vol, float pan) {
    if (name == nullptr) return;
    SimpleAudioEngine* audio = [SimpleAudioEngine sharedEngine];
    NSString* ns_name = [NSString stringWithUTF8String: name];
    [audio playEffect: ns_name loop: FALSE pitch: 1.0 pan: pan gain: vol];
}

void AudioMini::play_music(const char* name, float vol) {
    if (name == nullptr) return;
    SimpleAudioEngine* audio = [SimpleAudioEngine sharedEngine];
    NSString* ns_name = [NSString stringWithUTF8String: name];
    if (![audio isBackgroundMusicPlaying]) {
        [audio playBackgroundMusic: ns_name loop: TRUE];
    }
    [audio setBackgroundMusicVolume: vol];
}

void AudioMini::vibrate(int duration_millis) {
    if (duration_millis <= 0) return;
    AudioServicesPlayAlertSound(kSystemSoundID_Vibrate);
}

}
