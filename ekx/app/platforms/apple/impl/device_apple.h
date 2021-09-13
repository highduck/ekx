#pragma once

#include <TargetConditionals.h>

#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>
#import <AudioToolbox/AudioToolbox.h>

namespace ek::app {

const char* getPreferredLang() {
    NSString* language = [NSLocale preferredLanguages][0];
    if (language != nil) {
        static char lang[8];
        lang[0] = '\0';
        strncat(lang, [language UTF8String], 7);
        return lang;
    }
    return nullptr;
}

const float* getScreenInsets() {
#if TARGET_OS_IOS || TARGET_OS_TV
    if (@available(iOS 11.0, *)) {
        UIView *view = gAppDelegate.view;
        CGFloat scaleFactor = view.contentScaleFactor;
        UIEdgeInsets safeAreaInsets = view.safeAreaInsets;
        static float pads[4];
        pads[0] = (float)(safeAreaInsets.left * scaleFactor);
        pads[1] = (float)(safeAreaInsets.top * scaleFactor);
        pads[2] = (float)(safeAreaInsets.right * scaleFactor);
        pads[3] = (float)(safeAreaInsets.bottom * scaleFactor);
        return pads;
    }
#endif
    return nullptr;
}

void vibrate(int millis) {
    (void) (millis);
    AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}

const char* getSystemFontPath(const char* fontName) {
    CFStringRef targetName = CFStringCreateWithCString(nullptr, fontName, kCFStringEncodingUTF8);
    CTFontDescriptorRef targetDescriptor = CTFontDescriptorCreateWithNameAndSize(targetName, 0.0);
    CFURLRef targetURL = (CFURLRef) CTFontDescriptorCopyAttribute(targetDescriptor, kCTFontURLAttribute);
    if (targetURL) {
        static char path[PATH_MAX];
        CFURLGetFileSystemRepresentation(targetURL, true, reinterpret_cast<UInt8*>(path), PATH_MAX);
        CFRelease(targetURL);
        return path;
    }

    CFRelease(targetName);
    CFRelease(targetDescriptor);

    return nullptr;
}

}
