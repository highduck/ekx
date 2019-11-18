#include <ek/mini_ads.hpp>

#include <AppDelegate.h>

#import <StoreKit/StoreKit.h>
#import <GoogleMobileAds/GoogleMobileAds.h>

NSString* _GAD_app;
NSString* _GAD_banner;
NSString* _GAD_video;
NSString* _GAD_inters;
NSString* _GAD_remove_ads_sku;

GADBannerView* _bannerView;
GADInterstitial* _interstitial;

using ::ek::AdsEventType;

// fwd

namespace ek {
void __post_application_event(AdsEventType type);
}
void remove_ads_purchase_aknowledged();

// ios application spec
void justInfoMessage(NSString* title, NSString* buttonText) {
    UIAlertController * alert = [UIAlertController
            alertControllerWithTitle:title
                             message:nil
                      preferredStyle:UIAlertControllerStyleAlert];

    UIAlertAction* button = [UIAlertAction
            actionWithTitle:buttonText
                      style:UIAlertActionStyleCancel
                    handler:^(UIAlertAction * action) {
                        //Handle your yes please button action here
                    }];

    [alert addAction:button];

    [appDelegate.window.rootViewController presentViewController:alert animated:YES completion:nil];
}

/** AdMob **/
void addBannerViewToView(GADBannerView* bannerView) {
    UIViewController* root_view_controller = appDelegate.window.rootViewController;
    UIView* view = root_view_controller.view;
    bannerView.translatesAutoresizingMaskIntoConstraints = NO;
    bannerView.rootViewController = root_view_controller;
    bannerView.adUnitID = _GAD_banner;
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

/*** Purchases ***/
NSArray *Store_validProducts = nil;
bool Store_adsRemoved = false;

@interface PaymentTransactionObserver : NSObject<SKPaymentTransactionObserver>
@end

@implementation PaymentTransactionObserver

- (void) paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue
{
    NSLog(@"received restored transactions: %lu", (unsigned long)queue.transactions.count);
    for(SKPaymentTransaction *transaction in queue.transactions){
        if(transaction.transactionState == SKPaymentTransactionStateRestored){
            //called when the user successfully restores a purchase
            NSLog(@"Transaction state -> Restored");

            //if you have more than one in-app purchase product,
            //you restore the correct product for the identifier.
            //For example, you could use
            //if(productID == kRemoveAdsProductIdentifier)
            //to get the product identifier for the
            //restored purchases, you can use
            //
            //NSString *productID = transaction.payment.productIdentifier;
            [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
            break;
        }
    }
}

- (void)paymentQueue:(nonnull SKPaymentQueue *)queue updatedTransactions:(nonnull NSArray<SKPaymentTransaction *> *)transactions {
    for (SKPaymentTransaction *transaction in transactions) {
        switch (transaction.transactionState) {
            case SKPaymentTransactionStatePurchasing:
                NSLog(@"Purchasing");
                break;

            case SKPaymentTransactionStatePurchased:
                if ([transaction.payment.productIdentifier
                        isEqualToString:_GAD_remove_ads_sku]) {
                    remove_ads_purchase_aknowledged();
                    justInfoMessage(@"Purchase is completed succesfully! Enjoy playing with no ads", @"Cool!");
                }
                [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
                break;

            case SKPaymentTransactionStateRestored:
                if ([transaction.payment.productIdentifier
                        isEqualToString:_GAD_remove_ads_sku]) {
                    remove_ads_purchase_aknowledged();
                    justInfoMessage(@"Your Purchase is restored succesfully! Enjoy playing with no ads", @"Cool!");
                }
                [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
                break;

            case SKPaymentTransactionStateFailed:
                justInfoMessage(@"Purchase transaction has been failed", @"Continue");
                [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
                break;
            default:
                break;
        }
    }
}

@end

@interface ProductRequestDelegate : NSObject<SKProductsRequestDelegate>

@end

@implementation ProductRequestDelegate

- (void)productsRequest:(nonnull SKProductsRequest *)request didReceiveResponse:(nonnull SKProductsResponse *)response {
    SKProduct *validProduct = nil;
    NSUInteger count = [response.products count];

    if (count > 0) {
        Store_validProducts = [response.products copy];
        validProduct = [response.products objectAtIndex:0];
        if ([validProduct.productIdentifier isEqualToString:_GAD_remove_ads_sku]) {
            NSLog(@"Product Title: %@", validProduct.localizedTitle);
            NSLog(@"Product Desc: %@", validProduct.localizedDescription);
            NSLog(@"Product Price: %@", validProduct.price);
        }
    } else {
        //justInfoMessage(@"Currently Purchases are not available", @"Cancel");
        NSLog(@"Currently Purchases are not available");
    }

    //[activityIndicatorView stopAnimating];
    //purchaseButton.hidden = NO;
}

@end

ProductRequestDelegate* product_request_delegate;
PaymentTransactionObserver* payment_transaction_observer;

void init_payment_queue_observer() {
    if(!payment_transaction_observer) {
        payment_transaction_observer = [PaymentTransactionObserver new];
        [[SKPaymentQueue defaultQueue] addTransactionObserver:payment_transaction_observer];
    }
}

void ads_onRemoveAds() {
    Store_adsRemoved = true;
    __post_application_event(AdsEventType::ADS_REMOVED);
}

void ads_remove_save() {
    [[NSUserDefaults standardUserDefaults] setInteger:310 forKey:@"noads"];
    [[NSUserDefaults standardUserDefaults] setInteger:1988 forKey:@"_bm9hZHM="];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

void ads_remove() {
    _GAD_banner = nil;
    _GAD_inters = nil;
    if(_bannerView != nil) {
        _bannerView.hidden = YES;
        [_bannerView removeFromSuperview];
        _bannerView = nil;
    }
    ads_onRemoveAds();
}

void remove_ads_purchase_aknowledged() {
    ads_remove_save();
    ads_remove();
}

void Store_fetchAvailableProducts() {

    NSSet *productIdentifiers = [NSSet
            setWithObjects:_GAD_remove_ads_sku,nil];
    SKProductsRequest* productsRequest = [[SKProductsRequest alloc]
            initWithProductIdentifiers:productIdentifiers];
    if(!product_request_delegate) {
        product_request_delegate = [ProductRequestDelegate new];
    }
    productsRequest.delegate = product_request_delegate;
    [productsRequest start];
}

GADRequest* createAdRequest() {
    GADRequest* request = [GADRequest request];
    request.testDevices = @[
            kGADSimulatorID,
            @"3a65fb92e7f416b7f5c8aeef45f23f8c"
    ];
    return request;
}

void __reload_video_ad() {
    [[GADRewardBasedVideoAd sharedInstance] loadRequest:createAdRequest()
//                                           withAdUnitID:@"ca-app-pub-3940256099942544/1712485313"];
                                           withAdUnitID:_GAD_video];
}

@interface RewardVideoAdDelegate : NSObject<GADRewardBasedVideoAdDelegate>

@end

@implementation RewardVideoAdDelegate

- (void)rewardBasedVideoAd:(GADRewardBasedVideoAd *)rewardBasedVideoAd
   didRewardUserWithReward:(GADAdReward *)reward {
    //    NSString *rewardMessage =
    //    [NSString stringWithFormat:@"Reward received with currency %@ , amount %lf",
    //     reward.type,
    //     [reward.amount doubleValue]];
    __post_application_event(AdsEventType::ADS_VIDEO_REWARDED);
    NSLog(@"rewardMessage");
}

- (void)rewardBasedVideoAdDidReceiveAd:(GADRewardBasedVideoAd *)rewardBasedVideoAd {
    NSLog(@"Reward based video ad is received.");
    __post_application_event(AdsEventType::ADS_VIDEO_REWARD_LOADED);
}

- (void)rewardBasedVideoAdDidOpen:(GADRewardBasedVideoAd *)rewardBasedVideoAd {
    NSLog(@"Opened reward based video ad.");
}

- (void)rewardBasedVideoAdDidStartPlaying:(GADRewardBasedVideoAd *)rewardBasedVideoAd {
    NSLog(@"Reward based video ad started playing.");
}

- (void)rewardBasedVideoAdDidCompletePlaying:(GADRewardBasedVideoAd *)rewardBasedVideoAd {
    NSLog(@"Reward based video ad has completed.");
}

- (void)rewardBasedVideoAdDidClose:(GADRewardBasedVideoAd *)rewardBasedVideoAd {
    NSLog(@"Reward based video ad is closed.");
    __post_application_event(AdsEventType::ADS_VIDEO_REWARD_CLOSED);
    __reload_video_ad();
}

- (void)rewardBasedVideoAdWillLeaveApplication:(GADRewardBasedVideoAd *)rewardBasedVideoAd {
    NSLog(@"Reward based video ad will leave application.");
}

- (void)rewardBasedVideoAd:(GADRewardBasedVideoAd *)rewardBasedVideoAd
    didFailToLoadWithError:(NSError *)error {
    __post_application_event(AdsEventType::ADS_VIDEO_REWARD_FAIL);
    NSLog(@"Reward based video ad failed to load.");
}

@end

RewardVideoAdDelegate* reward_video_ad_delegate;

namespace ek {

// TODO: signals to core module
static std::function<void(AdsEventType type)> ads_registered_callbacks;
void __post_application_event(AdsEventType type) {
if(ads_registered_callbacks) {
ads_registered_callbacks(type);
}
}

/// interfaces

void ads_listen(const std::function<void(AdsEventType type)>& callback) {
ads_registered_callbacks = callback;
}

void init_billing(const char*) {

}

void ads_reset_purchase() {
    [[NSUserDefaults standardUserDefaults] setInteger:3109 forKey:@"noads"];
    [[NSUserDefaults standardUserDefaults] setInteger:19888 forKey:@"_bm9hZHM="];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

void ads_init(const ads_config_t& config) {
    // settings
    _GAD_app = [NSString stringWithUTF8String:config.app_id.c_str()];
    _GAD_banner = [NSString stringWithUTF8String:config.banner.c_str()];
    _GAD_video = [NSString stringWithUTF8String:config.video.c_str()];
    _GAD_inters = [NSString stringWithUTF8String:config.inters.c_str()];
    _GAD_remove_ads_sku = [NSString stringWithUTF8String:config.remove_ads_sku.c_str()];

    init_payment_queue_observer();

    if(!reward_video_ad_delegate) {
        reward_video_ad_delegate = [RewardVideoAdDelegate new];
    }
    [GADRewardBasedVideoAd sharedInstance].delegate = reward_video_ad_delegate;
    Store_fetchAvailableProducts();

    if ([[NSUserDefaults standardUserDefaults] integerForKey:@"noads"] == 310 &&
        [[NSUserDefaults standardUserDefaults] integerForKey:@"_bm9hZHM="] == 1988) {
        ads_remove();
    }

    [GADMobileAds configureWithApplicationID:_GAD_app];

    if (!Store_adsRemoved) {
        // banner
        // In this case, we instantiate the banner with desired ad size.
        _bannerView = [[GADBannerView alloc] initWithAdSize:kGADAdSizeBanner];
        addBannerViewToView(_bannerView);
        _bannerView.hidden = YES;

        // interstitial
        _interstitial = [[GADInterstitial alloc] initWithAdUnitID:_GAD_inters];
        [_interstitial loadRequest:createAdRequest()];
    }

    // video
    __reload_video_ad();
}

void ads_set_banner(int flags) {
    if(_bannerView != nil && _GAD_banner != nil && _GAD_banner.length > 0) {
        if(flags != 0) {
            [_bannerView loadRequest:createAdRequest()];
            _bannerView.hidden = NO;
        }
        else {
            _bannerView.hidden = YES;
        }
    }
}

void ads_play_reward_video() {
    UIViewController* root_view_controller = appDelegate.window.rootViewController;
    if ([[GADRewardBasedVideoAd sharedInstance] isReady]) {
        [[GADRewardBasedVideoAd sharedInstance] presentFromRootViewController:root_view_controller];
    }
    else {
        __reload_video_ad();
        __post_application_event(AdsEventType::ADS_VIDEO_LOADING);
    }
}

void ads_show_interstitial() {
    if (_GAD_inters == nil) {
        NSLog(@"[Interstitial] Ads removed");
        return;
    }

    if(_interstitial != nil && _interstitial.isReady) {
        UIViewController* root_view_controller = appDelegate.window.rootViewController;
        [_interstitial presentFromRootViewController:root_view_controller];
        // load next
        _interstitial = [[GADInterstitial alloc] initWithAdUnitID:_GAD_inters];
        [_interstitial loadRequest:[GADRequest request]];
    } else {
        NSLog(@"Ad wasn't ready");
    }
}

void ads_purchase_remove() {
    if (![SKPaymentQueue canMakePayments]) {
        justInfoMessage(@"Purchases are disabled in your device", @"Cancel");
        return;
    }

    UIAlertController * alert = [UIAlertController
            alertControllerWithTitle:@"Remove Ads"
                             message:@"Remove banner and interstitial ads from the game"
//                                 preferredStyle:UIAlertControllerStyleActionSheet];
                      preferredStyle:UIAlertControllerStyleAlert];

    SKProduct* product = nil;
    NSString* localized_price = @"";
    NSString* buyText = @"Buy";

    if(Store_validProducts != nil) {
        product = [Store_validProducts objectAtIndex:0];
        NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
        [numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
        [numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
        [numberFormatter setLocale:product.priceLocale];
        localized_price = [numberFormatter stringFromNumber:product.price];
        buyText = [NSString stringWithFormat:@"Buy %@", localized_price];
    }

    UIAlertAction* purchaseButton = [UIAlertAction
            actionWithTitle:buyText
                      style:UIAlertActionStyleDefault
                    handler:^(UIAlertAction * action) {
                        if(product != nil) {
                            SKPayment *payment = [SKPayment paymentWithProduct:product];
                            init_payment_queue_observer();
                            [[SKPaymentQueue defaultQueue] addPayment:payment];
                        }
                        else {
                            justInfoMessage(@"Sorry! Purchase is not available right now. Try Again later", @"Cancel");
                        }
                        //Handle your yes please button action here
                    }];

    UIAlertAction* restoreButton = [UIAlertAction
            actionWithTitle:@"Restore"
                      style:UIAlertActionStyleDefault
                    handler:^(UIAlertAction * action) {
                        init_payment_queue_observer();
                        [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
                    }];

    UIAlertAction* cancelButton = [UIAlertAction
            actionWithTitle:@"Cancel"
                      style:UIAlertActionStyleCancel
                    handler:^(UIAlertAction * action) {

                    }];

    [alert addAction:purchaseButton];
    [alert addAction:restoreButton];
    [alert addAction:cancelButton];

//    if(alert.popoverPresentationController != nil) {
//        alert.popoverPresentationController.sourceView = _rootViewController.view;
//        CGRect bounds = _rootViewController.view.bounds;
//        alert.popoverPresentationController.sourceRect = bounds;
//
//                                                                    bounds.origin.x + bounds.size.width * 0.5f,
//                                                                    bounds.origin.y + bounds.size.height* 0.5f,
//                                                                    0.0f, 0.0f);
//    }

    UIViewController* root_view_controller = appDelegate.window.rootViewController;
    [root_view_controller presentViewController:alert animated:YES completion:nil];
//
//    if(Store_validProducts != nil) {
//
//    }
//    else {
//
//         // REMOVE ME!!!
////        ads_remove();
//    }
}
}
