#ifdef __APPLE__

#include <ek/app_native.h>
#include <ek/assert.h>

// for system font
#include <CoreText/CoreText.h>

#if TARGET_OS_IOS || TARGET_OS_TV

#include "apple_ios.m.h"

#else

#include "apple_macos.m.h"

#endif

AppDelegate* ek_app_delegate = NULL;

#ifndef EK_NO_MAIN

int main(int argc, char* argv[]) {
    ek_app.argc = argc;
    ek_app.argv = argv;
    ek_app__init();

    NSString* language = [NSLocale preferredLanguages][0];
    if (language != nil) {
        strncat(ek_app.lang, [language UTF8String], sizeof(ek_app.lang));
    }

    @autoreleasepool {
#if TARGET_OS_IOS || TARGET_OS_TV
        UIApplicationMain(argc, argv, nil, NSStringFromClass(AppDelegate.class));
#else
        [NSApplication.sharedApplication setDelegate:[AppDelegate new]];
        [NSApplication.sharedApplication run];
#endif
    }

    return 0;
}

#endif

int ek_app_open_url(const char* url) {
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

int ek_app_font_path(char* dest, uint32_t size, const char* font_name) {
    int status = 1;
    CFStringRef targetName = CFStringCreateWithCString(NULL, font_name, kCFStringEncodingUTF8);
    CTFontDescriptorRef targetDescriptor = CTFontDescriptorCreateWithNameAndSize(targetName, 0.0);
    CFURLRef targetURL = (CFURLRef) CTFontDescriptorCopyAttribute(targetDescriptor, kCTFontURLAttribute);
    if (targetURL) {
        CFURLGetFileSystemRepresentation(targetURL, true, (UInt8*) dest, size);
        CFRelease(targetURL);
        status = 0;
    }
    CFRelease(targetName);
    CFRelease(targetDescriptor);
    return status;
}

int ek_app_share(const char* content) {
#if TARGET_OS_IOS || TARGET_OS_TV
    NSString* ns_text = [NSString stringWithUTF8String:content];
    NSArray* sharedObjects = @[ns_text];
    UIActivityViewController* vcActivity = [[UIActivityViewController alloc] initWithActivityItems:sharedObjects applicationActivities:nil];
    UIViewController* vcRoot = ek_app_delegate.window.rootViewController;
    vcActivity.popoverPresentationController.sourceView = vcRoot.view;
    [vcRoot presentViewController:vcActivity animated:YES completion:nil];
    return 0;
#else
    (void) sizeof(content);
    return 1;
#endif
}

void* ek_app_mtl_device(void) {
    if (ek_app_delegate.view != nil) {
        return (__bridge void*) (ek_app_delegate.view.device);
    }
    return NULL;
}

const void* ek_app_mtl_render_pass(void) {
    if (ek_app_delegate.view != nil) {
        return (__bridge const void*) (ek_app_delegate.view.defaultPass);
    }
    return NULL;
}

const void* ek_app_mtl_drawable(void) {
    if (ek_app_delegate.view != nil) {
        return (__bridge const void*) (ek_app_delegate.view.currentDrawable);
    }
    return NULL;
}

// just C version could be found here: https://stackoverflow.com/questions/20283054/xcode-file-open-native-c
const char* ek_app_ns_bundle_path(const char* path, char* buffer, uint32_t size) {
    EK_ASSERT(path != 0);
    EK_ASSERT(buffer != 0);
    NSString* ns_path = [NSString stringWithUTF8String:path];
    NSString* ns_bundle_path = [[NSBundle mainBundle] pathForResource:ns_path ofType:nil];
    if(ns_bundle_path && [ns_bundle_path getCString: buffer maxLength: size encoding: NSASCIIStringEncoding]) {
        return buffer;
    }
    return path;
}

#else // __APPLE__

/**
 * `.m` and `.mm` files should be ignored by build system for non-apple targets
 */

#endif // !__APPLE__
