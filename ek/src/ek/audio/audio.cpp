#include "audio.hpp"

#include <raudio/raudio.h>
#include <raudio/external/dr_mp3.h>

#include <unordered_map>
#include <string>

#include <ek/app/res.hpp>
#include <ek/util/logger.hpp>

// TODO: unload sound/music resources
// TODO: panning (left/right)
// TODO: rethink playing
// TODO: free music source buffer on unload (currently no unload methods)

static Wave LoadMP3(const ek::array_buffer& buffer, const char* optionalName) {
    Wave wave = {0};

    // Decode an entire MP3 file in one go
    uint64_t totalFrameCount = 0;
    drmp3_config config = {0};

    wave.data = drmp3_open_memory_and_read_pcm_frames_f32(
            buffer.data(),
            buffer.size(),
            &config,
            &totalFrameCount,
            NULL
    );

    wave.channels = config.outputChannels;
    wave.sampleRate = config.outputSampleRate;
    wave.sampleCount = (int) totalFrameCount * wave.channels;
    wave.sampleSize = 32;

    // NOTE: Only support up to 2 channels (mono, stereo)
    if (wave.channels > 2)
        EK_WARN("[%s] MP3 channels number (%i) not supported", optionalName, wave.channels);

    if (wave.data == NULL) {
        EK_WARN("[%s] MP3 data could not be loaded", optionalName);
    } else
        EK_INFO("[%s] MP3 file loaded successfully (%i Hz, %i bit, %s)",
                optionalName, wave.sampleRate, wave.sampleSize,
                (wave.channels == 1) ? "Mono" : "Stereo");

    return wave;
}


// Load music stream from file
Music LoadMusicStreamFromMemory(const ek::array_buffer& buffer, const char* fileName) {
    Music music = {0};
    bool musicLoaded = false;

    drmp3* ctxMp3 = (drmp3*) RL_MALLOC(sizeof(drmp3));
    music.ctxData = ctxMp3;
    music.ctxType = 3 /* MUSIC_AUDIO_MP3 */;
    auto* data = RL_MALLOC(buffer.size());
    memcpy(data, buffer.data(), buffer.size());
    // TODO: free data!

    int result = drmp3_init_memory(ctxMp3, data, buffer.size(), nullptr, nullptr);

    if (result > 0) {
        music.stream = InitAudioStream(ctxMp3->sampleRate, 32, ctxMp3->channels);
        music.sampleCount = drmp3_get_pcm_frame_count(ctxMp3) * ctxMp3->channels;
        music.looping = true;   // Infinite loop by default
        musicLoaded = true;
    }

    if (!musicLoaded) {
        drmp3_uninit((drmp3*) music.ctxData);
        RL_FREE(music.ctxData);

        EK_WARN("[%s] Music file could not be opened", fileName);
    } else {
        // Show some music stream info
        EK_INFO("[%s] Music file successfully loaded:", fileName);
        EK_INFO("   Total samples: %i", music.sampleCount);
        EK_INFO("   Sample rate: %i Hz", music.stream.sampleRate);
        EK_INFO("   Sample size: %i bits", music.stream.sampleSize);
        EK_INFO("   Channels: %i (%s)", music.stream.channels,
                (music.stream.channels == 1) ? "Mono" : (music.stream.channels == 2) ? "Stereo" : "Multi");
    }

    return music;
}

namespace ek::audio {

std::unordered_map<std::string, Sound> sounds;
std::unordered_map<std::string, Music> musics;

bool initialized = false;

void init() {
    assert(!initialized);
    InitAudioDevice();
    initialized = true;
}

void create_sound(const char* name) {
    if (name == nullptr) return;

    get_resource_content_async(name, [name](const array_buffer& buffer) {
        Wave wave = LoadMP3(buffer, name);
        sounds[name] = LoadSoundFromWave(wave);
    });
}

void create_music(const char* name) {
    if (name == nullptr) return;

    get_resource_content_async(name, [name](const array_buffer& buffer) {
        musics[name] = LoadMusicStreamFromMemory(buffer, name);
    });
}

void play_sound(const char* name, float vol) {
    if (name == nullptr) return;
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        const auto sound = it->second;
        SetSoundVolume(sound, vol);
        PlaySound(sound);
    }
}

void play_music(const char* name, float vol) {
    if (name == nullptr) return;
    auto it = musics.find(name);
    if (it != musics.end()) {
        const auto& music = it->second;
        if (!IsMusicPlaying(music)) {
            SetMusicVolume(music, vol);
            PlayMusicStream(music);
        } else {
            SetMusicVolume(music, vol);
            UpdateMusicStream(music);
        }
    }
}

int lockCounter = 0;

void muteDeviceBegin() {
    assert(lockCounter >= 0);
    if (lockCounter == 0) {
        //PauseDevice();
        SetMasterVolume(0.0f);
    }
    ++lockCounter;
}

void muteDeviceEnd() {
    --lockCounter;
    assert(lockCounter >= 0);
    if (lockCounter == 0) {
        // device could be paused during background
        //ResumeDevice();

        SetMasterVolume(1.0f);
    }
}

#if !defined(__APPLE__) && !defined(__ANDROID__)
void vibrate(int duration_millis) {

}
#endif
}