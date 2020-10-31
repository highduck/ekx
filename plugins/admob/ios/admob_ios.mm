#include <admob.hpp>
#include <ios_app_delegate.h>

#import <GoogleMobileAds/GoogleMobileAds.h>

namespace admob {

NSString* GAD_banner;
NSString* GAD_video;
NSString* GAD_inters;

GADBannerView* bannerView;
GADInterstitial* interstitial;
GADRewardedAd* rewardedAd;

/// AdMob
void addBannerViewToView(GADBannerView* bannerView) {
    UIViewController* root_view_controller = g_app_delegate.window.rootViewController;
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

void reloadRewardedAd() {
    if(rewardedAd == nil && GAD_video != nil && GAD_video.length > 0) {
        rewardedAd = [[GADRewardedAd alloc] initWithAdUnitID:GAD_video];
      [rewardedAd loadRequest:createAdRequest() completionHandler:^(GADRequestError * _Nullable error) {
        if (error) {
          // Handle ad failed to load case.
            onEvent(event_type::video_failed);
        } else {
          // Ad successfully loaded.
            onEvent(event_type::video_loaded);
        }
      }];
    }
}

}

@interface InterstitialAdDelegate : NSObject<GADInterstitialDelegate>
@end

@implementation InterstitialAdDelegate

/// Called when an interstitial ad request succeeded. Show it at the next transition point in your
/// application such as when transitioning between view controllers.
- (void)interstitialDidReceiveAd:(nonnull GADInterstitial *)ad {
    
}

/// Called when an interstitial ad request completed without an interstitial to
/// show. This is common since interstitials are shown sparingly to users.
- (void)interstitial:(nonnull GADInterstitial *)ad
    didFailToReceiveAdWithError:(nonnull GADRequestError *)error {
    admob::loadNextInterstitialAd();
}

/// Called just before presenting an interstitial. After this method finishes the interstitial will
/// animate onto the screen. Use this opportunity to stop animations and save the state of your
/// application in case the user leaves while the interstitial is on screen (e.g. to visit the App
/// Store from a link on the interstitial).
- (void)interstitialWillPresentScreen:(nonnull GADInterstitial *)ad {
    
}

/// Called when |ad| fails to present.
- (void)interstitialDidFailToPresentScreen:(nonnull GADInterstitial *)ad {
    admob::onInterstitialClosed();
    admob::loadNextInterstitialAd();
}

/// Called before the interstitial is to be animated off the screen.
- (void)interstitialWillDismissScreen:(nonnull GADInterstitial *)ad {
    
}

/// Called just after dismissing an interstitial and it has animated off the screen.
- (void)interstitialDidDismissScreen:(nonnull GADInterstitial *)ad {
    admob::onInterstitialClosed();
    admob::loadNextInterstitialAd();
}

/// Called just before the application will background or terminate because the user clicked on an
/// ad that will launch another application (such as the App Store). The normal
/// UIApplicationDelegate methods, like applicationDidEnterBackground:, will be called immediately
/// before this.
- (void)interstitialWillLeaveApplication:(nonnull GADInterstitial *)ad {
    
}

@end

@interface RewardedAdDelegate : NSObject<GADRewardedAdDelegate>

@end

@implementation RewardedAdDelegate

- (void)rewardedAd:(GADRewardedAd *)rewardedAd userDidEarnReward:(GADAdReward *)reward {
  admob::onEvent(admob::event_type::video_rewarded);
  NSLog(@"rewardedAd:userDidEarnReward:");
}

- (void)rewardedAdDidPresent:(GADRewardedAd *)rewardedAd {
  NSLog(@"rewardedAdDidPresent:");
}

- (void)rewardedAdDidDismiss:(GADRewardedAd *)rewardedAd {
  NSLog(@"rewardedAdDidDismiss:");
    admob::onEvent(admob::event_type::video_closed);
    admob::reloadRewardedAd();
}

- (void)rewardedAd:(GADRewardedAd *)rewardedAd didFailToPresentWithError:(NSError *)error {
  NSLog(@"rewardedAd:didFailToPresentWithError");
    admob::onEvent(admob::event_type::video_failed);
    admob::reloadRewardedAd();
}

@end

namespace admob {

static RewardedAdDelegate *rewardedAdDelegate = [RewardedAdDelegate new];
static InterstitialAdDelegate *interstitialAdDelegate = [InterstitialAdDelegate new];
 
/// interfaces
void loadNextInterstitialAd() {
    interstitial = [[GADInterstitial alloc] initWithAdUnitID:GAD_inters];
    [interstitial setDelegate:interstitialAdDelegate];
    [interstitial loadRequest:[GADRequest request]];
}

void initialize(const config_t& config) {
    // settings
    GAD_banner = [NSString stringWithUTF8String:config.banner.c_str()];
    GAD_video = [NSString stringWithUTF8String:config.video.c_str()];
    GAD_inters = [NSString stringWithUTF8String:config.inters.c_str()];

    GADMobileAds.sharedInstance.requestConfiguration.testDeviceIdentifiers = @[
            kGADSimulatorID,
            @"3a65fb92e7f416b7f5c8aeef45f23f8c"
    ];
    
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
        onEvent(event_type::initialized);

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

void show_rewarded_ad() {
    UIViewController* root_view_controller = g_app_delegate.window.rootViewController;
    if (rewardedAd.isReady) {
      [rewardedAd presentFromRootViewController:root_view_controller delegate:rewardedAdDelegate];
    } else {
      NSLog(@"Ad wasn't ready");
        reloadRewardedAd();
        onEvent(event_type::video_loading);
    }
}

void show_interstitial_ad() {
    if (GAD_inters == nil) {
        NSLog(@"[Interstitial] Ads removed");
        return;
    }

    if(interstitial != nil && interstitial.isReady) {
        UIViewController* root_view_controller = g_app_delegate.window.rootViewController;
        [interstitial presentFromRootViewController:root_view_controller];
    } else {
        NSLog(@"Ad wasn't ready");
    }
}

bool hasSupport() {
    return true;
}

}
