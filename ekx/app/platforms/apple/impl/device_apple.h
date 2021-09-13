#pragma once

#include <TargetConditionals.h>

#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>
#import <AudioToolbox/AudioToolbox.h>

namespace ek::app {

int openURL(const char* url) {
    NSString* str = [NSString stringWithUTF8String:url];
    NSURL* URL = [NSURL URLWithString:str];
#if TARGET_OS_IOS || TARGET_OS_TV
    UIApplication* application = [UIApplication sharedApplication];
    if ([application respondsToSelector:@selector(openURL:options:completionHandler:)]) {
        [application openURL:URL options:@{} completionHandler:^(BOOL success) {
            //NSLog(@"Open %@: %d", URL, success);
        }];
        return 0;
    } else {
        return [application openURL:URL] ? 0 : -1;
    }
#else
    return [[NSWorkspace sharedWorkspace] openURL:URL] ? 0 : -1;
#endif
}

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

int vibrate(int millis) {
    (void) (millis);
    AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
    return true;
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
