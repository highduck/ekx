#include <ek/app_native.h>

#import <GoogleMobileAds/GoogleMobileAds.h>
#import <AppTrackingTransparency/AppTrackingTransparency.h>

static NSString* GAD_banner = nil;
static NSString* GAD_video = nil;
static NSString* GAD_inters = nil;

static GADBannerView* bannerView = nil;
static GADInterstitialAd* interstitialAd = nil;
static GADRewardedAd* rewardedAd = nil;

/// AdMob
void addBannerViewToView(GADBannerView* bannerView) {
    UIViewController* root_view_controller = ek_app_delegate.window.rootViewController;
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

GADRequest* createAdRequest(void) {
    GADRequest* request = [GADRequest request];
    //    request.testDevices = @[
    //            kGADSimulatorID,
    //            @"3a65fb92e7f416b7f5c8aeef45f23f8c"
    //    ];
    return request;
}

void loadNextInterstitialAd(void);

void reloadRewardedAd(void);

@interface InterstitialAdDelegate : NSObject<GADFullScreenContentDelegate>
@end

@implementation InterstitialAdDelegate

- (void)adDidPresentFullScreenContent:(id)ad {
    NSLog(@"Interstitial Ad did present full screen content.");
}

- (void)ad:(id)ad didFailToPresentFullScreenContentWithError:(NSError *)error {
    NSLog(@"Interstitial Ad failed to present full screen content with error %@.", [error localizedDescription]);
    ek_admob__post(EK_ADMOB_INTERSTITIAL_CLOSED);;
    loadNextInterstitialAd();
}

- (void)adDidDismissFullScreenContent:(id)ad {
    NSLog(@"Interstitial Ad did dismiss full screen content.");
    ek_admob__post(EK_ADMOB_INTERSTITIAL_CLOSED);;
    loadNextInterstitialAd();
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
    ek_admob__post(EK_ADMOB_VIDEO_FAILED);
    //reloadRewardedAd();
}

- (void)adDidDismissFullScreenContent:(id)ad {
    NSLog(@"Rewarded ad dismissed.");
    ek_admob__post(EK_ADMOB_VIDEO_CLOSED);
}

@end

static RewardedAdDelegate *rewardedAdDelegate = nil;
static InterstitialAdDelegate *interstitialAdDelegate = nil;

/// interfaces
void loadNextInterstitialAd(void) {
    if(GAD_inters == nil || GAD_inters.length == 0) {
        return;
    }
    [GADInterstitialAd loadWithAdUnitID: GAD_inters
    request: createAdRequest()
    completionHandler: ^(GADInterstitialAd *ad, NSError *error) {
        if (error) {
            NSLog(@"Failed to load interstitial ad with error: %@", [error localizedDescription]);
            interstitialAd = nil;
            ek_admob__post(EK_ADMOB_INTERSTITIAL_CLOSED);
            // do not load until next show request
            //loadNextInterstitialAd();
            return;
        }
        interstitialAd = ad;
        interstitialAd.fullScreenContentDelegate = interstitialAdDelegate;
    }];
}

void configureChildDirected(ek_admob_child_directed child_directed) {
    if(child_directed == EK_ADMOB_CHILD_DIRECTED_UNSPECIFIED) {
        return;
    }
    [GADMobileAds.sharedInstance.requestConfiguration tagForChildDirectedTreatment: (child_directed == EK_ADMOB_CHILD_DIRECTED_TRUE ? TRUE : FALSE)];
}

void ek_admob_init(ek_admob_config config) {
    ek_admob__init();
    ek_admob.config = config;

    rewardedAdDelegate = [RewardedAdDelegate new];
    interstitialAdDelegate = [InterstitialAdDelegate new];
    
    // settings
    GAD_banner = config.banner ? [NSString stringWithUTF8String:config.banner] : nil;
    GAD_video = config.video ? [NSString stringWithUTF8String:config.video] : nil;
    GAD_inters = config.inters ? [NSString stringWithUTF8String:config.inters] : nil;

    GADMobileAds.sharedInstance.requestConfiguration.testDeviceIdentifiers = @[
            kGADSimulatorID,
            @"3a65fb92e7f416b7f5c8aeef45f23f8c"
            ];

    configureChildDirected(config.child_directed);

    [GADMobileAds.sharedInstance startWithCompletionHandler:^(GADInitializationStatus *status){

        // banner
        // In this case, we instantiate the banner with desired ad size.
        bannerView = [[GADBannerView alloc] initWithAdSize:kGADAdSizeBanner];
        addBannerViewToView(bannerView);
        bannerView.hidden = YES;

        if (@available(iOS 14.0, *))
        {
            [ATTrackingManager requestTrackingAuthorizationWithCompletionHandler:^(ATTrackingManagerAuthorizationStatus status) {
                loadNextInterstitialAd();
                reloadRewardedAd();
                ek_admob__post(EK_ADMOB_INITIALIZED);
            }];
        }
        else {
            loadNextInterstitialAd();
            reloadRewardedAd();
            ek_admob__post(EK_ADMOB_INITIALIZED);
        }
    }];
}

void ek_admob_show_banner(int flags) {
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

void reloadRewardedAd(void) {
    if(GAD_video == nil || GAD_video.length == 0) {
        ek_admob__post(EK_ADMOB_VIDEO_FAILED);
        return;
    }
    [GADRewardedAd loadWithAdUnitID: GAD_video
    request: createAdRequest()
    completionHandler: ^(GADRewardedAd *ad, NSError *error) {
        if (error) {
            NSLog(@"%@", error.debugDescription);
            // Handle ad failed to load case.
            ek_admob__post(EK_ADMOB_VIDEO_FAILED);
        } else {
            rewardedAd = ad;
            rewardedAd.fullScreenContentDelegate = rewardedAdDelegate;
            // Ad successfully loaded.
            ek_admob__post(EK_ADMOB_VIDEO_LOADED);
        }
    }];
}

bool isRewardedAdReady(void) {
    if(rewardedAd != nil) {
        UIViewController* rootViewController = ek_app_delegate.window.rootViewController;
        if([rewardedAd canPresentFromRootViewController: rootViewController error:nil]) {
            return true;
        }
    }
    return false;
}

void ek_admob_show_rewarded_ad(void) {
    if (GAD_video == nil) {
        NSLog(@"Rewarded Ad is not configured");
        return;
    }

    if (isRewardedAdReady()) {
        UIViewController* rootViewController = ek_app_delegate.window.rootViewController;
        [rewardedAd presentFromRootViewController: rootViewController
        userDidEarnRewardHandler:^ {
            NSLog(@"rewardedAd:userDidEarnReward:");
            ek_admob__post(EK_ADMOB_VIDEO_REWARDED);
        }];
    } else {
        NSLog(@"Ad wasn't ready");
        reloadRewardedAd();
        ek_admob__post(EK_ADMOB_VIDEO_LOADING);
    }
}

static bool isInterstitialAdReady(void) {
    if(interstitialAd != nil) {
        UIViewController* rootViewController = ek_app_delegate.window.rootViewController;
        if([interstitialAd canPresentFromRootViewController: rootViewController error:nil]) {
            return true;
        }
    }

    return false;
}

void ek_admob_show_interstitial_ad(void) {
    if (GAD_inters == nil) {
        NSLog(@"Interstitial Ad is not configured or removed by purchase");
        return;
    }

    if(isInterstitialAdReady()) {
        UIViewController* rootViewController = ek_app_delegate.window.rootViewController;
        [interstitialAd presentFromRootViewController: rootViewController];
    } else {
        NSLog(@"Interstitial Ad is not ready");
        ek_admob__post(EK_ADMOB_INTERSTITIAL_CLOSED);
        if (interstitialAd == nil) {
            loadNextInterstitialAd();
        }
    }
}

bool ek_admob_supported(void) {
    return true;
}
