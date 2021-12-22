#ifndef AUPH_NATIVE_DEVICE_OBOE_IMPL
#define AUPH_NATIVE_DEVICE_OBOE_IMPL
#else
#error You should implement auph once
#endif

#include "native.h"
#include <oboe/Oboe.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>

#include <stdio.h>
#include <math.h>

#ifndef OBOE_NULL

// implement oboe-lib:
#include <oboe-all.cpp>

#endif

#if defined(NDEBUG)

#define AUPH_ALOGV(...)
#define AUPH_ALOGD(...)
#define AUPH_ALOGI(...)
#define AUPH_ALOGW(...)
#define AUPH_ALOGE(...)
#define AUPH_ALOGF(...)
#define AUPH_AASSERT(cond, ...)

#else

#define AUPH_ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE,"AUPH",__VA_ARGS__)
#define AUPH_ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG,"AUPH",__VA_ARGS__)
#define AUPH_ALOGI(...) __android_log_print(ANDROID_LOG_INFO,"AUPH",__VA_ARGS__)
#define AUPH_ALOGW(...) __android_log_print(ANDROID_LOG_WARN,"AUPH",__VA_ARGS__)
#define AUPH_ALOGE(...) __android_log_print(ANDROID_LOG_ERROR,"AUPH",__VA_ARGS__)
#define AUPH_ALOGF(...) __android_log_print(ANDROID_LOG_FATAL,"AUPH",__VA_ARGS__)
#define AUPH_AASSERT(cond, ...) if (!(cond)) {__android_log_assert(#cond,"AUPH",__VA_ARGS__);}

#endif

typedef JNIEnv* (* auph_android_get_jni_env)(void);

inline static struct {
    jobject assets_ref = nullptr;
    AAssetManager* assets = nullptr;
    jobject activity = nullptr;
    auph_android_get_jni_env get_jni_env = nullptr;
} auph_android;

void auph_android_setup(auph_android_get_jni_env get_jni_env, jobject activity, jobject assets_ref) {
    auph_android.get_jni_env = get_jni_env;
    if (get_jni_env) {
        JNIEnv* env = get_jni_env();
        if (env) {
            if (activity) {
                auph_android.activity = env->NewGlobalRef(activity);
            }
            if (assets_ref) {
                auph_android.assets_ref = env->NewGlobalRef(assets_ref);
                if (auph_android.assets_ref) {
                    auph_android.assets = AAssetManager_fromJava(env, auph_android.assets_ref);
                }
            }
        }
    }
}

#ifndef OBOE_NULL

void auph_oboe_open_stream(auph_audio_device* device);

bool auph_oboe_reopen_and_start_stream(auph_audio_device* device);

struct auph_audio_device {
    auph_audio_stream_info playbackStreamInfo;
    auph_audio_device_callback onPlayback;
    void* userData;
    oboe::AudioStream* audioStream;
    class auph_oboe_callback* oboe_callback;
};

class auph_oboe_callback :
        public oboe::AudioStreamDataCallback,
        public oboe::AudioStreamErrorCallback {
public:
    bool onError(oboe::AudioStream* stream, oboe::Result error) override {
        (void) stream;
        (void) error;
        return false;
    }

    void onErrorBeforeClose(oboe::AudioStream* stream, oboe::Result error) override {
        (void) stream;
        (void) error;
    }

    void onErrorAfterClose(oboe::AudioStream* stream, oboe::Result error) override {
        (void) stream;
        auph_audio_device* device = auph_get_audio_device();
        // Restart the stream if the error is a disconnect, otherwise do nothing and log the error
        // reason.
        if (error == oboe::Result::ErrorDisconnected) {
            AUPH_ALOGI("AudioStream disconnected. Restart if is active currently");
            if (device->audioStream) {
                AUPH_ALOGI("Restart disconnected AudioStream, because stream should be active");
                auph_oboe_reopen_and_start_stream(device);
            }
        } else {
            AUPH_ALOGE("Error was %s", oboe::convertToText(error));
        }
    }

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream* stream, void* audioData, int32_t numFrames) override {
        // prevent OpenSLES case, `audioData` could be null or no frames are required
        // https://github.com/google/oboe/issues/559
        if (audioData == nullptr || numFrames <= 0) {
            return oboe::DataCallbackResult::Continue;
        }
        auph_audio_device* device = auph_get_audio_device();
        if (device->onPlayback) {
            auph_audio_callback_data data;
            data.data = audioData;
            data.stream = device->playbackStreamInfo;
            data.userData = device->userData;
            data.frames = numFrames;
            device->onPlayback(&data);
        } else {
            memset(audioData, 0, numFrames * stream->getBytesPerFrame());
        }
        return oboe::DataCallbackResult::Continue;
    }
};

void auph_oboe_open_stream(auph_audio_device* device) {
    device->audioStream = nullptr;
    //std::lock_guard<std::mutex> lock(mLock);
    oboe::AudioStreamBuilder builder{};
    //builder.setAudioApi(oboe::AudioApi::OpenSLES);
    builder.setDirection(oboe::Direction::Output);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
    builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setFormat(oboe::AudioFormat::I16);
    builder.setChannelCount(oboe::ChannelCount::Stereo);
    builder.setDataCallback(device->oboe_callback);
    builder.setErrorCallback(device->oboe_callback);
    //builder.setFramesPerDataCallback(128);

    oboe::Result result = builder.openStream(&device->audioStream);
    if (result != oboe::Result::OK) {
        AUPH_ALOGE("Failed to create stream. Error: %s", oboe::convertToText(result));
        device->audioStream = nullptr;
        return;
    }

    device->playbackStreamInfo.bytesPerFrame = device->audioStream->getBytesPerFrame();
    device->playbackStreamInfo.bytesPerSample = device->audioStream->getBytesPerSample();
    device->playbackStreamInfo.sampleRate = (float) device->audioStream->getSampleRate();
    device->playbackStreamInfo.channels = device->audioStream->getChannelCount();
    device->playbackStreamInfo.format = AUPH_SAMPLE_FORMAT_I16;
}

bool auph_oboe_reopen_and_start_stream(auph_audio_device* device) {
    auph_oboe_open_stream(device);
    if (device->audioStream == nullptr) {
        return false;
    }
    const oboe::Result result = device->audioStream->requestStart();
    if (result != oboe::Result::OK) {
        AUPH_ALOGE("Error starting playback stream after disconnection. Error: %s",
                   oboe::convertToText(result));
        device->audioStream->close();
        device->audioStream = nullptr;
        return false;
    }
    return true;
}

void auph_audio_device_init(auph_audio_device* device) {
    memset(device, 0, sizeof(auph_audio_device));
    device->oboe_callback = new auph_oboe_callback();
}

bool auph_audio_device_start(auph_audio_device* device) {
    auph_oboe_open_stream(device);
    if (device->audioStream != nullptr) {
        oboe::Result result = device->audioStream->requestStart();
        if (result != oboe::Result::OK) {
            AUPH_ALOGE("Error starting playback stream. Error: %s", oboe::convertToText(result));
            device->audioStream->close();
            device->audioStream = nullptr;
            return false;
        }

        if (auph_android.get_jni_env && auph_android.activity) {
            JNIEnv* env = auph_android.get_jni_env();
            if (env) {
                jclass cls = env->FindClass("ek/Auph");
                if (cls) {
                    jmethodID fn = env->GetStaticMethodID(cls, "start",
                                                          "(Landroid/app/Activity;)V");
                    if (fn) {
                        env->CallStaticVoidMethod(cls, fn, auph_android.activity);
                    } else {
                        AUPH_ALOGE("Error cannot get start() function");
                    }
                } else {
                    AUPH_ALOGE("Error cannot find Auph class");
                }
            }
        }
        return true;
    }
    return false;
}

bool auph_audio_device_stop(auph_audio_device* device) {
    if (device->audioStream != nullptr) {
        if (auph_android.get_jni_env && auph_android.activity) {
            JNIEnv* env = auph_android.get_jni_env();
            if (env) {
                jclass cls = env->FindClass("ek/Auph");
                if (cls) {
                    jmethodID fn = env->GetStaticMethodID(cls, "stop",
                                                          "(Landroid/app/Activity;)V");
                    if (fn) {
                        env->CallStaticVoidMethod(cls, fn, auph_android.activity);
                    } else {
                        AUPH_ALOGE("Error cannot get stop() function");
                    }
                } else {
                    AUPH_ALOGE("Error cannot find Auph class");
                }
            }
        }

        device->audioStream->close();
        device->audioStream = nullptr;
        return true;
    }
    return false;
}

void auph_audio_device_term(auph_audio_device* device) {
    device->userData = NULL;
    device->onPlayback = NULL;
    auph_audio_device_stop(device);
    delete device->oboe_callback;
}

int auph_vibrate(int millis) {
    int result = 1;
    if (auph_android.get_jni_env && auph_android.activity) {
        JNIEnv* env = auph_android.get_jni_env();
        if (env) {
            jclass cls = env->FindClass("ek/Auph");
            if (cls) {
                jmethodID mid = env->GetStaticMethodID(cls, "vibrate",
                                                       "(Landroid/app/Activity;I)I");
                if (mid) {
                    result = env->CallStaticIntMethod(cls, mid, auph_android.activity, millis);
                } else {
                    AUPH_ALOGE("Error cannot get vibrate() function");
                }
                env->DeleteLocalRef(cls);
            } else {
                AUPH_ALOGE("Error cannot find Auph class");
            }
        }
    }
    return result;
}

#endif

extern "C" JNIEXPORT JNICALL
jint Java_ek_Auph_restart(JNIEnv* env, jclass clazz) {
    (void) env;
    (void) clazz;
#ifndef OBOE_NULL
    auph_audio_device* device = auph_get_audio_device();
    if (device) {
        oboe::AudioStream* stream = device->audioStream;
        if (stream != nullptr) {
            stream->close();
            if (auph_oboe_reopen_and_start_stream(device)) {
                return 0;
            }
            return 1;
        }
        return 2;
    }
    return 3;
#else
    return 0;
#endif
}

