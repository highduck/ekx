#pragma once

#include <string>

#include <TargetConditionals.h>

#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>
#import <AudioToolbox/AudioToolbox.h>

namespace ek {

std::string get_device_lang() {
    std::string result = "en-US";
    NSString* language = [NSLocale preferredLanguages][0];
    if(language != nil) {
        result = [language UTF8String];
    }

    return result;
}

void getScreenInsets(float padding[4]) {
    padding[0] = 0.0f;
    padding[1] = 0.0f;
    padding[2] = 0.0f;
    padding[3] = 0.0f;

#if TARGET_OS_IOS || TARGET_OS_TV
    if (@available(iOS 11.0, *)) {
        UIView *view = gAppDelegate.view;
        CGFloat scaleFactor = view.contentScaleFactor;
        UIEdgeInsets safeAreaInsets = view.safeAreaInsets;
        padding[0] = (float)(safeAreaInsets.left * scaleFactor);
        padding[1] = (float)(safeAreaInsets.top * scaleFactor);
        padding[2] = (float)(safeAreaInsets.right * scaleFactor);
        padding[3] = (float)(safeAreaInsets.bottom * scaleFactor);
    }
#endif
}

void vibrate(int duration_millis) {
    (void) (duration_millis);
    AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}

std::string getDeviceFontPath(const char* fontName) {
    CFStringRef targetName = CFStringCreateWithCString(nullptr, fontName, kCFStringEncodingUTF8);
    CTFontDescriptorRef targetDescriptor = CTFontDescriptorCreateWithNameAndSize(targetName, 0.0);
    CFURLRef targetURL = (CFURLRef) CTFontDescriptorCopyAttribute(targetDescriptor, kCTFontURLAttribute);
    std::string fontPath;

    if (targetURL) {
        UInt8 buffer[PATH_MAX];
        CFURLGetFileSystemRepresentation(targetURL, true, buffer, PATH_MAX);
        fontPath = std::string((char*) buffer);
        CFRelease(targetURL);
    }

    CFRelease(targetName);
    CFRelease(targetDescriptor);

    return fontPath;
}

}
