#include <admob.hpp>
#include <app_apple.h>

#import <GoogleMobileAds/GoogleMobileAds.h>

namespace admob {

NSString* GAD_banner;
NSString* GAD_video;
NSString* GAD_inters;

GADBannerView* bannerView = nil;
GADInterstitialAd* interstitialAd = nil;
GADRewardedAd* rewardedAd = nil;

/// AdMob
void addBannerViewToView(GADBannerView* bannerView) {
    UIViewController* root_view_controller = gAppDelegate.window.rootViewController;
    UIView* view = root_view_controller.view;
    bannerView.translatesAutoresizingMaskIntoConstraints = NO;
    bannerView.rootViewController = root_view_controller;
    bannerView.adUnitID = GAD_banner;
    [view addSubview:bannerView];
    [view addConstraints:@[
            [NSLayoutConstraint constraintWithItem:bannerView
                                         attribute:NSLayoutAttributeBottom
                                         relatedBy:NSLayoutRelationEqual
                                            toItem:root_view_controller.bottomLayoutGuide
                                         attribute:NSLayoutAttributeTop
                                        multiplier:1
                                          constant:0],
            [NSLayoutConstraint constraintWithItem:bannerView
                                         attribute:NSLayoutAttributeCenterX
                                         relatedBy:NSLayoutRelationEqual
                                            toItem:view
                                         attribute:NSLayoutAttributeCenterX
                                        multiplier:1
                                          constant:0]
    ]];
}

GADRequest* createAdRequest() {
    GADRequest* request = [GADRequest request];
//    request.testDevices = @[
//            kGADSimulatorID,
//            @"3a65fb92e7f416b7f5c8aeef45f23f8c"
//    ];
    return request;
}

void loadNextInterstitialAd();

void reloadRewardedAd();

}
@interface InterstitialAdDelegate : NSObject<GADFullScreenContentDelegate>
@end

@implementation InterstitialAdDelegate

- (void)adDidPresentFullScreenContent:(id)ad {
    NSLog(@"Interstitial Ad did present full screen content.");
}

- (void)ad:(id)ad didFailToPresentFullScreenContentWithError:(NSError *)error {
    NSLog(@"Interstitial Ad failed to present full screen content with error %@.", [error localizedDescription]);
    admob::context.onInterstitialClosed();
    admob::loadNextInterstitialAd();
}

- (void)adDidDismissFullScreenContent:(id)ad {
    NSLog(@"Interstitial Ad did dismiss full screen content.");
    admob::context.onInterstitialClosed();
    admob::loadNextInterstitialAd();
}

@end

@interface RewardedAdDelegate : NSObject<GADFullScreenContentDelegate>

@end

@implementation RewardedAdDelegate

- (void)adDidPresentFullScreenContent:(id)ad {
    NSLog(@"Rewarded ad presented");
}

- (void)ad:(id)ad didFailToPresentFullScreenContentWithError:(NSError *)error {
    NSLog(@"Rewarded ad failed to present with error: %@", [error localizedDescription]);
    admob::context.onEvent(admob::event_type::video_failed);
    //admob::reloadRewardedAd();
}

- (void)adDidDismissFullScreenContent:(id)ad {
    NSLog(@"Rewarded ad dismissed.");
    admob::context.onEvent(admob::event_type::video_closed);
}

@end

namespace admob {

static RewardedAdDelegate *rewardedAdDelegate = [RewardedAdDelegate new];
static InterstitialAdDelegate *interstitialAdDelegate = [InterstitialAdDelegate new];
 
/// interfaces
void loadNextInterstitialAd() {
    if(GAD_inters == nil || GAD_inters.length == 0) {
        return;
    }
    [GADInterstitialAd loadWithAdUnitID: GAD_inters
                     request: createAdRequest()
                     completionHandler: ^(GADInterstitialAd *ad, NSError *error) {
          if (error) {
              NSLog(@"Failed to load interstitial ad with error: %@", [error localizedDescription]);
              interstitialAd = nil;
              context.onInterstitialClosed();
              // do not load until next show request
              //loadNextInterstitialAd();
              return;
          }
          interstitialAd = ad;
          interstitialAd.fullScreenContentDelegate = interstitialAdDelegate;
    }];
}

void configureChildDirected(ChildDirected childDirected) {
    if(childDirected == ChildDirected::Unspecified) {
        return;
    }
    [GADMobileAds.sharedInstance.requestConfiguration
        tagForChildDirectedTreatment: (childDirected == ChildDirected::True ? TRUE : FALSE)
    ];
}

void initialize(const config_t& config) {
    _initialize();

    // settings
    GAD_banner = [NSString stringWithUTF8String:config.banner.c_str()];
    GAD_video = [NSString stringWithUTF8String:config.video.c_str()];
    GAD_inters = [NSString stringWithUTF8String:config.inters.c_str()];
    

    GADMobileAds.sharedInstance.requestConfiguration.testDeviceIdentifiers = @[
            kGADSimulatorID,
            @"3a65fb92e7f416b7f5c8aeef45f23f8c"
    ];
    
    configureChildDirected(config.childDirected);
    
    [GADMobileAds.sharedInstance startWithCompletionHandler:^(GADInitializationStatus *status){
        
        // banner
        // In this case, we instantiate the banner with desired ad size.
        bannerView = [[GADBannerView alloc] initWithAdSize:kGADAdSizeBanner];
        addBannerViewToView(bannerView);
        bannerView.hidden = YES;

        // interstitial
        loadNextInterstitialAd();

        // video
        reloadRewardedAd();
        
        context.onEvent(event_type::initialized);
    }];
}

void show_banner(int flags) {
    if(bannerView != nil && GAD_banner != nil && GAD_banner.length > 0) {
        if(flags != 0) {
            [bannerView loadRequest:createAdRequest()];
            bannerView.hidden = NO;
        }
        else {
            bannerView.hidden = YES;
        }
    }
}

void reloadRewardedAd() {
    if(GAD_video == nil || GAD_video.length == 0) {
        return;
    }
    [GADRewardedAd loadWithAdUnitID: GAD_video
              request: createAdRequest()
              completionHandler: ^(GADRewardedAd *ad, NSError *error) {
        if (error) {
            NSLog(@"%@", error.debugDescription);
            // Handle ad failed to load case.
            context.onEvent(event_type::video_failed);
        } else {
            rewardedAd = ad;
            rewardedAd.fullScreenContentDelegate = rewardedAdDelegate;
            // Ad successfully loaded.
            context.onEvent(event_type::video_loaded);
        }
    }];
}

bool isRewardedAdReady() {
    if(rewardedAd != nil) {
        UIViewController* rootViewController = gAppDelegate.window.rootViewController;
        if([rewardedAd canPresentFromRootViewController: rootViewController error:nil]) {
            return true;
        }
    }
    return false;
}

void show_rewarded_ad() {
    if (GAD_video == nil) {
        NSLog(@"Rewarded Ad is not configured");
        return;
    }
    
    if (isRewardedAdReady()) {
        UIViewController* rootViewController = gAppDelegate.window.rootViewController;
        [rewardedAd presentFromRootViewController: rootViewController
                userDidEarnRewardHandler:^ {
                        NSLog(@"rewardedAd:userDidEarnReward:");
                        context.onEvent(admob::event_type::video_rewarded);
                }];
    } else {
      NSLog(@"Ad wasn't ready");
        reloadRewardedAd();
        context.onEvent(event_type::video_loading);
    }
}

bool isInterstitialAdReady() {
    if(interstitialAd != nil) {
        UIViewController* rootViewController = gAppDelegate.window.rootViewController;
        if([interstitialAd canPresentFromRootViewController: rootViewController error:nil]) {
            return true;
        }
    }
    
    return false;
}
    
void show_interstitial_ad() {
    if (GAD_inters == nil) {
        NSLog(@"Interstitial Ad is not configured or removed by purchase");
        return;
    }
    
    if(isInterstitialAdReady()) {
        UIViewController* rootViewController = gAppDelegate.window.rootViewController;
        [interstitialAd presentFromRootViewController: rootViewController];
    } else {
        NSLog(@"Interstitial Ad is not ready");
        context.onInterstitialClosed();
        if (interstitialAd == nil) {
            loadNextInterstitialAd();
        }
    }
}

bool hasSupport() {
    return true;
}

}
