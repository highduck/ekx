#pragma once

#if TARGET_OS_IOS || TARGET_OS_TV
#import <UIKit/UIKit.h>
#import <Firebase/Firebase.h>
#endif

#include <ek/app/app.hpp>

namespace ek {

void sharing_rate_us(const char* appID) {
    //action=write-review
    // 1435111697
    char buffer[4096];
    snprintf(buffer, 4096, "itms-apps://itunes.apple.com/us/app/apple-store/id%s?mt=8&action=write-review", appID);
    app::openURL(buffer);

#if TARGET_OS_IOS || TARGET_OS_TV
    [FIRAnalytics logEventWithName:kFIREventViewItem
    parameters:@{kFIRParameterItemID: @"app_store"}];
#endif
}

void sharing_send_message(const char* text) {
    (void) text;
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
