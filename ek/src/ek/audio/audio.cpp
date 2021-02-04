#include "audio.hpp"

#define DR_MP3_NO_STDIO

#define MA_NO_JACK
#define MA_NO_WAV
#define MA_NO_FLAC

#define MA_NO_ENCODING
#define MA_NO_STDIO
//#define MA_NO_GENERATION

// android issue
#define MA_NO_AAUDIO
#define MA_NO_SDL
#define MA_NO_RUNTIME_LINKING

#define MINIAUDIO_IMPLEMENTATION

#include <cstdio>

#define audio_on_assert(e, file, line) ((void)printf("%s:%d: failed assertion `%s'\n", file, line, e), abort())
#define audio_assert(e) (!(e) ? audio_on_assert(#e, __FILE__, __LINE__) : ((void)0))

#define MA_ASSERT(e) audio_assert(e)

#include <miniaudio.h>
#include <miniaudio_engine.h>

#include <cassert>

#include <ek/app/res.hpp>
#include <ek/util/logger.hpp>

namespace ek::audio {

struct AudioSystem {
    ma_engine engine;
    ma_resource_manager* pResourceManager;
    ma_sound_group soundsGroup;
    ma_sound_group musicGroup;
    int locks = 0;
    bool initialized = false;
};

static AudioSystem audioSystem{};

void init() {
    assert(!audioSystem.initialized);
    {
        ma_result result = ma_engine_init(NULL, &audioSystem.engine);
        if (result != MA_SUCCESS) {
            EK_WARN("Failed to initialize audio engine.");
            return;
        }
        audioSystem.pResourceManager = audioSystem.engine.pResourceManager;

        result = ma_sound_group_init(&audioSystem.engine, 0, NULL, &audioSystem.soundsGroup);
        if (result != MA_SUCCESS) {
            EK_WARN("Failed to create sound group");
            return;
        }

        result = ma_sound_group_init(&audioSystem.engine, 0, NULL, &audioSystem.musicGroup);
        if (result != MA_SUCCESS) {
            EK_WARN("Failed to create sound group");
            return;
        }
    }
    audioSystem.initialized = true;
}

void muteDeviceBegin() {
    assert(audioSystem.locks >= 0);
    if (audioSystem.locks == 0) {
        //PauseDevice();
        ma_engine_set_volume(&audioSystem.engine, 0.0f);
    }
    ++audioSystem.locks;
}

void muteDeviceEnd() {
    --audioSystem.locks;
    assert(audioSystem.locks >= 0);
    if (audioSystem.locks == 0) {
        // device could be paused during background
        //ResumeDevice();
        ma_engine_set_volume(&audioSystem.engine, 1.0f);
    }
}

#if !defined(__APPLE__) && !defined(__ANDROID__)
void vibrate(int duration_millis) {

}
#endif

/** Sound **/

void Sound::load(const char* path) {
    dataSourceFilePath = path;
    get_resource_content_async(path, [this](array_buffer&& buffer) {
        source = std::move(buffer);

        ma_resource_manager_memory_buffer memoryBuffer;
        memoryBuffer.encoded.pData = source.data();
        memoryBuffer.encoded.sizeInBytes = source.size();
        ma_result result = ma_resource_manager_register_data(
                audioSystem.pResourceManager,
                dataSourceFilePath.c_str(),
                nullptr,
                ma_resource_manager_data_buffer_encoding_encoded,
                &memoryBuffer);
        if (result != MA_SUCCESS) {
            EK_WARN("cannot register data");
        }
        dataSource = new ma_resource_manager_data_source();
        result = ma_resource_manager_data_source_init(
                audioSystem.pResourceManager, dataSourceFilePath.c_str(),
                MA_DATA_SOURCE_FLAG_DECODE, nullptr, dataSource);
        if (result != MA_SUCCESS) {
            EK_WARN("cannot init stream data source");
        }
    });
}

void Sound::unload() {
    if (dataSource) {
        for (auto* sound : sounds) {
            if (sound) {
                ma_sound_uninit(sound);
                delete sound;
            }
        }
        sounds.resize(0);

        ma_resource_manager_unregister_data(audioSystem.pResourceManager, dataSourceFilePath.c_str());
        ma_resource_manager_data_source_uninit(dataSource);
        delete dataSource;
        dataSource = nullptr;
    }
}

Sound::~Sound() {
    unload();
}

void Sound::play(float volume, float pitch) {
    if (dataSource && volume > 0.0f) {
        auto* sound = getNextSound();
        if (sound) {
            ma_sound_set_volume(sound, volume);
            ma_sound_set_pitch(sound, pitch);
            ma_sound_start(sound);
        }
    }
}

ma_sound* Sound::getNextSound() {
    for (auto* snd : sounds) {
        if (snd && ma_sound_at_end(snd)) {
            if (ma_sound_is_playing(snd)) {
                ma_sound_stop(snd);
            }
//            ma_result result = ma_data_source_seek_to_pcm_frame(snd->pDataSource, 0);
//            if (result != MA_SUCCESS) {
//                return result;  /* Failed to seek back to the start. */
//            }
            return snd;
        }
    }
    auto* sound = new ma_sound();
//    auto result = ma_sound_init_from_data_source(&audioSystem.engine, dataSource, MA_DATA_SOURCE_FLAG_DECODE,
//                                            &audioSystem.soundsGroup, sound);
    auto result = ma_sound_init_from_file(&audioSystem.engine, dataSourceFilePath.c_str(), MA_DATA_SOURCE_FLAG_DECODE,
                                          &audioSystem.soundsGroup, sound);
    if (result != MA_SUCCESS) {
        EK_WARN("cannot init next sound: %s", dataSourceFilePath.c_str());
        delete sound;
        sound = nullptr;
    }
    else {
        sounds.push_back(sound);
    }
    return sound;
}

Sound::Sound(const char* path) {
    load(path);
}

/** Music **/

void Music::load(const char* path) {
    get_resource_content_async(path, [this, path](array_buffer buf) {
        source = std::move(buf);

        dataSourceFilePath = path;

        ma_resource_manager_memory_buffer memoryBuffer;
        memoryBuffer.encoded.pData = source.data();
        memoryBuffer.encoded.sizeInBytes = source.size();
        ma_result result = ma_resource_manager_register_data(
                audioSystem.pResourceManager,
                dataSourceFilePath.c_str(),
                nullptr,
                ma_resource_manager_data_buffer_encoding_encoded,
                &memoryBuffer);
        if (result != MA_SUCCESS) {
            EK_WARN("cannot register data");
        }
        dataSource = new ma_resource_manager_data_source();
        result = ma_resource_manager_data_source_init(
                audioSystem.pResourceManager, path,
                MA_DATA_SOURCE_FLAG_STREAM, NULL, dataSource);
        if (result != MA_SUCCESS) {
            EK_WARN("cannot init stream data source");
        }

        sound = new ma_sound();
        result = ma_sound_init_from_file(&audioSystem.engine, dataSourceFilePath.c_str(), MA_DATA_SOURCE_FLAG_STREAM,
                                            &audioSystem.musicGroup, sound);
        ma_sound_set_looping(sound, true);
        if (result != MA_SUCCESS) {
            EK_WARN("cannot init music");
        }
    });
}

void Music::unload() {
    if (dataSource) {
        ma_sound_uninit(sound);
        delete sound;
        sound = nullptr;

        ma_resource_manager_unregister_data(audioSystem.pResourceManager, dataSourceFilePath.c_str());
        ma_resource_manager_data_source_uninit(dataSource);
        delete dataSource;
        dataSource = nullptr;
    }
    source.resize(0);
    source.shrink_to_fit();
}

Music::~Music() {
    unload();
}

void Music::play() {
    if (sound) {
        if (!ma_sound_is_playing(sound)) {
            ma_sound_start(sound);
        }
    }
}

void Music::stop() {
    if (sound) {
        if (ma_sound_is_playing(sound)) {
            ma_sound_stop(sound);
        }
    }
}

void Music::update() {
    if (sound) {
//        ma_sound_update
//        ::Music music = *ptrHandle;
//        UpdateMusicStream(music);
    }
}

void Music::setVolume(float volume) {
    volume_ = volume;
    if (sound) {
        ma_sound_set_volume(sound, volume);
    }
}

float Music::getVolume() const {
    return volume_;
}

void Music::setPitch(float pitch) {
    pitch_ = pitch;
    if (sound) {
        ma_sound_set_pitch(sound, pitch);
    }
}

float Music::getPitch() const {
    return pitch_;
}

Music::Music(const char* path) {
    load(path);
}

}