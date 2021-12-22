#pragma once

#include "AudioDevice.hpp"
#include <TargetConditionals.h>
#import <AudioToolbox/AudioToolbox.h>

#if TARGET_OS_IOS
#include <AVFoundation/AVFoundation.h>
#endif

#if __has_feature(objc_arc)
#define AUPH_OBJC_RELEASE(obj) { obj = nil; }
#else
#define AUPH_OBJC_RELEASE(obj) { [obj release]; obj = nil; }
#endif

#if TARGET_OS_IOS
@interface AudioAppEventsHandler : NSObject {}
- (void)startInterruptionHandler;
- (void)stopInterruptionHandler;
@end

AudioAppEventsHandler* auph_audioAppEventsHandler = NULL;
#endif

void audioPlaybackCallback(void* inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer);

bool createAudioQueue(AudioQueueRef* outAudioQueue, auph_audio_device* device);

bool checkError(OSStatus status) {
    if (status != noErr) {
        // NSLog(@"Error: %d", status);
        return true;
    }
    return false;
}

enum {
    AUPH_AUDIO_DEVICE_BUFFER_FRAMES = 2048,
    AUPH_AUDIO_DEVICE_BUFFER_MAX_COUNT = 3,
};

struct auph_audio_device {
    auph_audio_stream_info playbackStreamInfo;
    auph_audio_device_callback onPlayback;
    void* userData;

    AudioQueueRef audioQueue;
    AudioQueueBufferRef buffers[AUPH_AUDIO_DEVICE_BUFFER_MAX_COUNT];
};

void auph_audio_device_init(auph_audio_device*) {
#if TARGET_OS_IOS
    auph_audioAppEventsHandler = [AudioAppEventsHandler new];
#endif
}

bool auph_audio_device_start(auph_audio_device* device) {
    if (!createAudioQueue(&device->audioQueue, device)) {
        return false;
    }
    if (checkError(AudioQueueStart(device->audioQueue, NULL))) {
        checkError(AudioQueueDispose(device->audioQueue, false));
        device->audioQueue = NULL;
        return false;
    }

#if TARGET_OS_IOS
    [auph_audioAppEventsHandler startInterruptionHandler];
#endif

    return true;
}

bool auph_audio_device_stop(auph_audio_device* device) {
    if (device->audioQueue != NULL) {
        auto* queue = device->audioQueue;
        if (checkError(AudioQueueStop(queue, true))) {
            return false;
        }

        if (checkError(AudioQueueDispose(queue, false))) {
            //    return false;
        }

        device->audioQueue = NULL;
#if TARGET_OS_IOS
        [auph_audioAppEventsHandler stopInterruptionHandler];
#endif
    }
    return true;
}

void auph_audio_device_term(auph_audio_device* device) {
    auph_audio_device_stop(device);
#if TARGET_OS_IOS
    AUPH_OBJC_RELEASE(auph_audioAppEventsHandler);
#endif
    device->userData = NULL;
    device->onPlayback = NULL;
}

int auph_vibrate(int millis) {
    (void) (millis);
    // TODO: short-long
    AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
    return 0;
}

bool createAudioQueue(AudioQueueRef* outAudioQueue, auph_audio_device* device) {
    AudioStreamBasicDescription format;
    format.mSampleRate = 44100.0;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    format.mFramesPerPacket = 1;
    format.mChannelsPerFrame = 2;
    format.mBitsPerChannel = 8 * sizeof(int16_t);
    format.mBytesPerFrame = sizeof(int16_t) * format.mChannelsPerFrame;
    format.mBytesPerPacket = format.mBytesPerFrame * format.mFramesPerPacket;
    format.mReserved = 0;

    if (checkError(AudioQueueNewOutput(&format,
                                       audioPlaybackCallback,
                                       device,
                                       NULL,
                                       NULL /* kCFRunLoopCommonModes */,
                                       0,
                                       outAudioQueue))) {
        return false;
    }

    /* create 3 audio buffers */
    for (int i = 0; i < AUPH_AUDIO_DEVICE_BUFFER_MAX_COUNT; ++i) {
        AudioQueueBufferRef buffer = NULL;
        const uint32_t bufferSize = AUPH_AUDIO_DEVICE_BUFFER_FRAMES * format.mBytesPerFrame;
        if (checkError(AudioQueueAllocateBuffer(*outAudioQueue, bufferSize, &buffer))) {
            return false;
        }
        buffer->mAudioDataByteSize = bufferSize;
        memset(buffer->mAudioData, 0, bufferSize);
        device->buffers[i] = buffer;
        AudioQueueEnqueueBuffer(*outAudioQueue, buffer, 0, NULL);
    }

    device->playbackStreamInfo.bytesPerSample = format.mBytesPerPacket;
    device->playbackStreamInfo.channels = format.mChannelsPerFrame;
    device->playbackStreamInfo.bytesPerFrame = format.mBytesPerFrame;
    device->playbackStreamInfo.sampleRate = (float) format.mSampleRate;
    device->playbackStreamInfo.format = AUPH_SAMPLE_FORMAT_I16;

    return true;
}

inline void audioPlaybackCallback(void* inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
    const auph_audio_device* device = (const auph_audio_device*)inUserData;
    auph_audio_device_callback cb = device->onPlayback;
    if (cb != NULL) {
        auph_audio_callback_data data;
        data.data = inBuffer->mAudioData;
        data.stream = device->playbackStreamInfo;
        data.userData = device->userData;
        data.frames = inBuffer->mAudioDataByteSize / data.stream.bytesPerFrame;
        cb(&data);
    } else {
        memset(inBuffer->mAudioData, 0, inBuffer->mAudioDataByteSize);
    }
    AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
}

#if TARGET_OS_IOS

@implementation AudioAppEventsHandler

-(id)init {
    self = [super init];

    //[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResign:) name:APPLE_ApplicationWillResignActiveNotification object:NULL];
    //[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive:) name:APPLE_ApplicationDidBecomeActiveNotification object:NULL];

    #if TARGET_OS_IOS
    AVAudioSession* session = [AVAudioSession sharedInstance];
    assert(session != nil);
    if(![session setCategory: AVAudioSessionCategoryAmbient
                           error:nil]) {
        NSLog(@"failed to activate audio session");
    }
    if(![session setActive:true error:nil]) {
        NSLog(@"failed to activate audio session");
    }
    #endif
    return self;
}

-(void)dealloc {
    //[[NSNotificationCenter defaultCenter] removeObserver:self name:APPLE_ApplicationWillResignActiveNotification object:NULL];
    //[[NSNotificationCenter defaultCenter] removeObserver:self name:APPLE_ApplicationDidBecomeActiveNotification object:NULL];

#if !__has_feature(objc_arc)
    [super dealloc];
#endif
}

-(void)startInterruptionHandler {
    AVAudioSession* session = [AVAudioSession sharedInstance];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(onInterruption:) name:AVAudioSessionInterruptionNotification object:session];
}

-(void)stopInterruptionHandler {
    [[NSNotificationCenter defaultCenter] removeObserver:self name:AVAudioSessionInterruptionNotification object:nil];
}

-(void)onInterruption:(NSNotification*)notification {
    auph_audio_device* device = auph_get_audio_device();
    NSLog(@"handle_interruption");
    AVAudioSession* session = [AVAudioSession sharedInstance];
    NSAssert(session, @"invalid audio session");
    NSDictionary* dict = notification.userInfo;
    NSAssert(dict, @"audio session user info missing");
    NSInteger type = [[dict valueForKey:AVAudioSessionInterruptionTypeKey] integerValue];
    NSInteger interruptionOption = [[dict valueForKey:AVAudioSessionInterruptionOptionKey] integerValue];
    switch (type) {
        case AVAudioSessionInterruptionTypeBegan:
            if(device != NULL) {
                auph_audio_device_stop(device);
            }
            [session setActive:false error:nil];
            break;
        case AVAudioSessionInterruptionTypeEnded:
            if (interruptionOption == AVAudioSessionInterruptionOptionShouldResume) {
                [session setActive:true error:nil];
                if(device != NULL) {
                    auph_audio_device_start(device);
                }
            }
            break;
        default:
            break;
    }
}

//-(void)applicationWillResign:(NSNotification*)notification {
//    auph_get_audio_device()->stop();
//}
//
//-(void)applicationDidBecomeActive:(NSNotification*)notification {
//    auph_get_audio_device()->start();
//}

@end

#endif