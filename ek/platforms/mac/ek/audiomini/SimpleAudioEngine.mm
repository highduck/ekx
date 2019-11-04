/****************************************************************************
Copyright (c) 2010 cocos2d-x.org
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "SimpleAudioEngine.h"
#include "SimpleAudioEngine_objc.h"
#include <string>

static NSFileManager* s_fileManager = [NSFileManager defaultManager];

bool isFileExistInternal(const std::string& filePath) {
    if (filePath.empty()) {
        return false;
    }

    bool ret = false;

    if (filePath[0] != '/') {
        std::string path;
        std::string file;
        size_t pos = filePath.find_last_of("/");
        if (pos != std::string::npos) {
            file = filePath.substr(pos + 1);
            path = filePath.substr(0, pos + 1);
        } else {
            file = filePath;
        }

        NSBundle* bundle = [NSBundle mainBundle];
        NSString* fullpath = [bundle pathForResource:[NSString stringWithUTF8String:file.c_str()]
                                              ofType:nil
                                         inDirectory:[NSString stringWithUTF8String:path.c_str()]];
        if (fullpath != nil) {
            ret = true;
        }
    } else {
        // Search path is an absolute path.
        if ([s_fileManager fileExistsAtPath:[NSString stringWithUTF8String:filePath.c_str()]]) {
            ret = true;
        }
    }

    return ret;
}


static std::string
getFullPathForFilenameWithinDirectory(const std::string& directory, const std::string& filename) {
    // get directory+filename, safely adding '/' as necessary
    std::string ret = directory;
    if (!directory.empty() && directory[directory.size() - 1] != '/') {
        ret += '/';
    }
    ret += filename;
    // if the file doesn't exist, return an empty string
    if (!isFileExistInternal(ret)) {
        ret = "";
    }
    return ret;
}

static std::string getPathForFilename(const std::string& filename, const std::string& resolutionDirectory,
                                      const std::string& searchPath) {
    std::string file = filename;
    std::string file_path = "";
    size_t pos = filename.find_last_of("/");
    if (pos != std::string::npos) {
        file_path = filename.substr(0, pos + 1);
        file = filename.substr(pos + 1);
    }

    // searchPath + file_path + resourceDirectory
    std::string path = searchPath;
    path += file_path;
    path += resolutionDirectory;

    path = getFullPathForFilenameWithinDirectory(path, file);

    return path;
}

std::string fullPathForFilename(const std::string& filename) {
    NSString* assetPath = [[NSString alloc] initWithCString:filename.c_str() encoding:NSASCIIStringEncoding];
    const char* filePath = [[[NSBundle mainBundle] pathForResource:assetPath ofType:nil] cStringUsingEncoding:NSASCIIStringEncoding];
    if (filePath) return std::string{filePath};
    // The file wasn't found, return empty string.
    return "";
}

static void static_end() {
    [SimpleAudioEngine end];
}

static void static_preloadBackgroundMusic(const char* pszFilePath) {
    [[SimpleAudioEngine sharedEngine] preloadBackgroundMusic:[NSString stringWithUTF8String:pszFilePath]];
}

static void static_playBackgroundMusic(const char* pszFilePath, bool bLoop) {
    [[SimpleAudioEngine sharedEngine] playBackgroundMusic:[NSString stringWithUTF8String:pszFilePath] loop:bLoop];
}

static void static_stopBackgroundMusic() {
    [[SimpleAudioEngine sharedEngine] stopBackgroundMusic];
}

static void static_pauseBackgroundMusic() {
    [[SimpleAudioEngine sharedEngine] pauseBackgroundMusic];
}

static void static_resumeBackgroundMusic() {
    [[SimpleAudioEngine sharedEngine] resumeBackgroundMusic];
}

static void static_rewindBackgroundMusic() {
    [[SimpleAudioEngine sharedEngine] rewindBackgroundMusic];
}

static bool static_willPlayBackgroundMusic() {
    return [[SimpleAudioEngine sharedEngine] willPlayBackgroundMusic];
}

static bool static_isBackgroundMusicPlaying() {
    return [[SimpleAudioEngine sharedEngine] isBackgroundMusicPlaying];
}

static float static_getBackgroundMusicVolume() {
    return [[SimpleAudioEngine sharedEngine] backgroundMusicVolume];
}

static void static_setBackgroundMusicVolume(float volume) {
    volume = MAX(MIN(volume, 1.0), 0);
    [SimpleAudioEngine sharedEngine].backgroundMusicVolume = volume;
}

static float static_getEffectsVolume() {
    return [[SimpleAudioEngine sharedEngine] effectsVolume];
}

static void static_setEffectsVolume(float volume) {
    volume = MAX(MIN(volume, 1.0), 0);
    [SimpleAudioEngine sharedEngine].effectsVolume = volume;
}

static unsigned int
static_playEffect(const char* pszFilePath, bool bLoop, Float32 pszPitch, Float32 pszPan, Float32 pszGain) {
    return [[SimpleAudioEngine sharedEngine] playEffect:[NSString stringWithUTF8String:pszFilePath] loop:bLoop pitch:pszPitch pan:pszPan gain:pszGain];
}

static void static_stopEffect(int nSoundId) {
    [[SimpleAudioEngine sharedEngine] stopEffect:nSoundId];
}

static void static_preloadEffect(const char* pszFilePath) {
    [[SimpleAudioEngine sharedEngine] preloadEffect:[NSString stringWithUTF8String:pszFilePath]];
}

static void static_unloadEffect(const char* pszFilePath) {
    [[SimpleAudioEngine sharedEngine] unloadEffect:[NSString stringWithUTF8String:pszFilePath]];
}

static void static_pauseEffect(unsigned int uSoundId) {
    [[SimpleAudioEngine sharedEngine] pauseEffect:uSoundId];
}

static void static_pauseAllEffects() {
    [[SimpleAudioEngine sharedEngine] pauseAllEffects];
}

static void static_resumeEffect(unsigned int uSoundId) {
    [[SimpleAudioEngine sharedEngine] resumeEffect:uSoundId];
}

static void static_resumeAllEffects() {
    [[SimpleAudioEngine sharedEngine] resumeAllEffects];
}

static void static_stopAllEffects() {
    [[SimpleAudioEngine sharedEngine] stopAllEffects];
}

namespace CocosDenshion {

    static SimpleAudioEngine* s_pEngine;

    SimpleAudioEngine::SimpleAudioEngine() {

    }

    SimpleAudioEngine::~SimpleAudioEngine() {

    }

    SimpleAudioEngine* SimpleAudioEngine::getInstance() {
        if (!s_pEngine) {
            s_pEngine = new(std::nothrow) SimpleAudioEngine();
        }

        return s_pEngine;
    }

    void SimpleAudioEngine::end() {
        if (s_pEngine) {
            delete s_pEngine;
            s_pEngine = nullptr;
        }

        static_end();
    }

    void SimpleAudioEngine::preloadBackgroundMusic(const char* pszFilePath) {
        // Changing file path to full path
        std::string fullPath = fullPathForFilename(pszFilePath);
        static_preloadBackgroundMusic(fullPath.c_str());
    }

    void SimpleAudioEngine::playBackgroundMusic(const char* pszFilePath, bool bLoop) {
        // Changing file path to full path
        std::string fullPath = fullPathForFilename(pszFilePath);
        static_playBackgroundMusic(fullPath.c_str(), bLoop);
    }

    void SimpleAudioEngine::stopBackgroundMusic(bool bReleaseData) {
        static_stopBackgroundMusic();
    }

    void SimpleAudioEngine::pauseBackgroundMusic() {
        static_pauseBackgroundMusic();
    }

    void SimpleAudioEngine::resumeBackgroundMusic() {
        static_resumeBackgroundMusic();
    }

    void SimpleAudioEngine::rewindBackgroundMusic() {
        static_rewindBackgroundMusic();
    }

    bool SimpleAudioEngine::willPlayBackgroundMusic() {
        return static_willPlayBackgroundMusic();
    }

    bool SimpleAudioEngine::isBackgroundMusicPlaying() {
        return static_isBackgroundMusicPlaying();
    }

    float SimpleAudioEngine::getBackgroundMusicVolume() {
        return static_getBackgroundMusicVolume();
    }

    void SimpleAudioEngine::setBackgroundMusicVolume(float volume) {
        static_setBackgroundMusicVolume(volume);
    }

    float SimpleAudioEngine::getEffectsVolume() {
        return static_getEffectsVolume();
    }

    void SimpleAudioEngine::setEffectsVolume(float volume) {
        static_setEffectsVolume(volume);
    }

    unsigned int SimpleAudioEngine::playEffect(const char* pszFilePath, bool bLoop,
                                               float pitch, float pan, float gain) {
        // Changing file path to full path
        std::string fullPath = fullPathForFilename(pszFilePath);
        return static_playEffect(fullPath.c_str(), bLoop, pitch, pan, gain);
    }

    void SimpleAudioEngine::stopEffect(unsigned int nSoundId) {
        static_stopEffect(nSoundId);
    }

    void SimpleAudioEngine::preloadEffect(const char* pszFilePath) {
        // Changing file path to full path
        std::string fullPath = fullPathForFilename(pszFilePath);
        static_preloadEffect(fullPath.c_str());
    }

    void SimpleAudioEngine::unloadEffect(const char* pszFilePath) {
        // Changing file path to full path
        std::string fullPath = fullPathForFilename(pszFilePath);
        static_unloadEffect(fullPath.c_str());
    }

    void SimpleAudioEngine::pauseEffect(unsigned int uSoundId) {
        static_pauseEffect(uSoundId);
    }

    void SimpleAudioEngine::resumeEffect(unsigned int uSoundId) {
        static_resumeEffect(uSoundId);
    }

    void SimpleAudioEngine::pauseAllEffects() {
        static_pauseAllEffects();
    }

    void SimpleAudioEngine::resumeAllEffects() {
        static_resumeAllEffects();
    }

    void SimpleAudioEngine::stopAllEffects() {
        static_stopAllEffects();
    }
}
