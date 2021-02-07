#include <ek/assert.hpp>
#include <ek/util/detect_platform.hpp>

#include "audio.hpp"

#define DR_MP3_NO_STDIO

#define MA_NO_JACK
#define MA_NO_WAV
#define MA_NO_FLAC

#define MA_NO_ENCODING
#define MA_NO_STDIO
#define MA_NO_GENERATION

// android issue
#define MA_NO_AAUDIO

#if EK_WEB || EK_ANDROID || EK_IOS
#define MA_NO_RUNTIME_LINKING
#endif

#define MA_ASSERT(e) EK_ASSERT(e)

// debugging
//#define MA_LOG_LEVEL 4
//#define MA_DEBUG_OUTPUT
// debugging

#define MINIAUDIO_IMPLEMENTATION

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
        auto config = ma_engine_config_init_default();

//#if EK_WEB
//        auto rmConfig = ma_resource_manager_config_init();
//        rmConfig.jobThreadCount = 0;
//        rmConfig.flags = MA_RESOURCE_MANAGER_FLAG_NO_THREADING;
//
//        config.pResourceManager = (ma_resource_manager*) malloc(sizeof(ma_resource_manager));
//        memset(config.pResourceManager, 0, sizeof(ma_resource_manager));
//
//        ma_resource_manager_init(&rmConfig, config.pResourceManager);
//#endif

        ma_result result = ma_engine_init(&config, &audioSystem.engine);
        //ma_result result = ma_engine_init(NULL, &audioSystem.engine);
        if (result != MA_SUCCESS) {
            EK_WARN("Failed to initialize audio engine: %i", result);
            return;
        }
        audioSystem.pResourceManager = audioSystem.engine.pResourceManager;

        result = ma_sound_group_init(&audioSystem.engine, 0, NULL, &audioSystem.soundsGroup);
        if (result != MA_SUCCESS) {
            EK_WARN("Failed to create sound group: %i", result);
            return;
        }

        result = ma_sound_group_init(&audioSystem.engine, 0, NULL, &audioSystem.musicGroup);
        if (result != MA_SUCCESS) {
            EK_WARN("Failed to create sound group: %i", result);
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

    auto result = ma_sound_init_from_file(&audioSystem.engine, dataSourceFilePath.c_str(),
//                                          MA_SOUND_FLAG_NO_PITCH | MA_SOUND_FLAG_NO_SPATIALIZATION |
                                          MA_SOUND_FLAG_DECODE,
                                          &audioSystem.soundsGroup, sound);
    if (result != MA_SUCCESS) {
        EK_WARN("cannot init next sound: %s", dataSourceFilePath.c_str());
        delete sound;
        sound = nullptr;
    } else {
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