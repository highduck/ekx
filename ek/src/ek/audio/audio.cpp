#include "audio.hpp"

#include <raudio/raudio.h>
#include <dr_mp3.h>

#include <unordered_map>
#include <cassert>

#include <ek/app/res.hpp>
#include <ek/util/logger.hpp>

// TODO: panning (left/right)

static Wave LoadMP3(const void* data, size_t size, const char* optionalName) {
    Wave wave = {0};

    // Decode an entire MP3 file in one go
    drmp3_uint64 totalFrameCount = 0;
    drmp3_config config = {0};

    wave.data = drmp3_open_memory_and_read_pcm_frames_f32(
            data,
            size,
            &config,
            &totalFrameCount,
            NULL
    );

    wave.channels = config.channels;
    wave.sampleRate = config.sampleRate;
    wave.sampleCount = static_cast<unsigned int>(totalFrameCount * wave.channels);
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
Music LoadMusicStreamFromMemory(void* data, size_t size, const char* fileName) {
    Music music = {0};
    bool musicLoaded = false;

    drmp3* ctxMp3 = (drmp3*) RL_MALLOC(sizeof(drmp3));
    music.ctxData = ctxMp3;
    music.ctxType = 3 /* MUSIC_AUDIO_MP3 */;

    int result = drmp3_init_memory(ctxMp3, data, size, nullptr);

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

bool initialized = false;

void init() {
    assert(!initialized);
    InitAudioDevice();
    initialized = true;
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

/** Sound **/

void Sound::load(const char* path) {
    get_resource_content_async(path, [this, path](const array_buffer& buffer) {
        ::Wave wave = LoadMP3(buffer.data(), buffer.size(), path);
        ptrHandle = new ::Sound(LoadSoundFromWave(wave));
        UnloadWave(wave);
    });
}

void Sound::unload() {
    if (ptrHandle) {
        UnloadSound(*ptrHandle);
        delete ptrHandle;
        ptrHandle = nullptr;
    }
}

Sound::~Sound() {
    unload();
}

void Sound::play(float volume) {
    if (ptrHandle && volume > 0.0f) {
        ::Sound handle = *ptrHandle;
        SetSoundVolume(handle, volume);
        PlaySound(handle);
    }
}

Sound::Sound(const char* path) {
    load(path);
}

/** Music **/

void Music::load(const char* path) {
    get_resource_content_async(path, [this, path](array_buffer buf) {
        ptrHandle = new ::Music(LoadMusicStreamFromMemory(buf.data(), buf.size(), path));
        source = std::move(buf);
    });
}

void Music::unload() {
    if (ptrHandle) {
        UnloadMusicStream(*ptrHandle);
        delete ptrHandle;
        ptrHandle = nullptr;
    }
    source.resize(0);
    source.shrink_to_fit();
}

Music::~Music() {
    unload();
}

void Music::play() {
    if (ptrHandle) {
        ::Music music = *ptrHandle;
        if (!IsMusicPlaying(music)) {
            PlayMusicStream(music);
        }
    }
}

void Music::stop() {
    if (ptrHandle) {
        ::Music music = *ptrHandle;
        if (IsMusicPlaying(music)) {
            StopMusicStream(music);
        }
    }
}

void Music::update() {
    if (ptrHandle) {
        ::Music music = *ptrHandle;
        UpdateMusicStream(music);
    }
}

void Music::setVolume(float volume) {
    volume_ = volume;
    if(ptrHandle) {
        SetMusicVolume(*ptrHandle, volume);
    }
}

float Music::getVolume() const {
    return volume_;
}

Music::Music(const char* path) {
    load(path);
}

}