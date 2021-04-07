#pragma once

#if TARGET_OS_IOS || TARGET_OS_TV
#import <UIKit/UIKit.h>
#import <Firebase/Firebase.h>
#endif

#include <string>

namespace ek {

static void ios_navigate(NSString* ns_url) {
    (void)ns_url;
#if TARGET_OS_IOS || TARGET_OS_TV
    NSURL* URL = [NSURL URLWithString:ns_url];
    UIApplication* application = [UIApplication sharedApplication];
    if ([application respondsToSelector:@selector(openURL:options:completionHandler:)]) {
        [application openURL:URL options:@{}
        completionHandler:^(BOOL success) {
        NSLog(@"Open %@: %d", ns_url, success);
    }];
    } else {
        BOOL success = [application openURL:URL];
        NSLog(@"Open %@: %d", ns_url, success);
    }
#endif
}

void sharing_navigate(const char* url) {
    NSString* ns_url = [NSString stringWithUTF8String:url];
    ios_navigate(ns_url);
}

void sharing_rate_us(const char* app_id) {
    //action=write-review
    // 1435111697
    NSString* ns_app_id = app_id != NULL ? [NSString stringWithUTF8String:app_id] : @"1435111697";
    NSString* url = [NSString stringWithFormat:@"itms-apps://itunes.apple.com/us/app/apple-store/id%@?mt=8&action=write-review", ns_app_id];

    ios_navigate(url);

#if TARGET_OS_IOS || TARGET_OS_TV
    [FIRAnalytics logEventWithName:kFIREventViewItem
    parameters:@{kFIRParameterItemID: @"app_store"}];
#endif
}

void sharing_send_message(const char* text) {
    (void)text;
#if TARGET_OS_IOS || TARGET_OS_TV
    NSString* ns_text = [NSString stringWithUTF8String:text];
    //NSURL *myWebsite = [NSURL URLWithString:@"itms://itunes.apple.com/us/app/apple-store/id375380948?mt=8"];
    //  UIImage * myImage =[UIImage imageNamed:@"myImage.png"];
    NSArray* sharedObjects = @[ns_text];
    UIActivityViewController* activityViewController = [[UIActivityViewController alloc] initWithActivityItems:sharedObjects applicationActivities:nil];

    UIViewController* root_view_controller = gAppDelegate.window.rootViewController;
    activityViewController.popoverPresentationController.sourceView = root_view_controller.view;
    [root_view_controller presentViewController:activityViewController animated:YES completion:nil];

    [FIRAnalytics logEventWithName:kFIREventShare
    parameters:@{kFIRParameterContentType: @"text"}];
#endif
}

}
