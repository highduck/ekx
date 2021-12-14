#include <GameServices.hpp>
#include <ek/app_native.h>
#include <ek/log.h>

#import <GameKit/GameKit.h>
#import <Firebase/Firebase.h>

BOOL GC_gameCenterEnabled = NO;
NSString* GC_leaderboardIdentifier;
UIViewController* GC_rootViewController;

@interface GameCenterDelegate: NSObject<GKGameCenterControllerDelegate>
@end

@implementation GameCenterDelegate
- (void)gameCenterViewControllerDidFinish:(GKGameCenterViewController*)gameCenterViewController {
    [GC_rootViewController dismissViewControllerAnimated:YES completion:nil];
}

@end

GameCenterDelegate* game_center_delegate;

/** Game Center **/
void GC_authenticateLocalPlayer() {
    game_center_delegate = [GameCenterDelegate new];
    GC_rootViewController = ek_app_delegate.window.rootViewController;
    GKLocalPlayer* localPlayer = [GKLocalPlayer localPlayer];

    localPlayer.authenticateHandler = ^(UIViewController* viewController, NSError* error) {
        if (viewController != nil) {
            [GC_rootViewController presentViewController:viewController animated:YES completion:nil];
        } else {
            if ([GKLocalPlayer localPlayer].authenticated) {
                GC_gameCenterEnabled = YES;

                // Get the default leaderboard identifier.
                [[GKLocalPlayer localPlayer] loadDefaultLeaderboardIdentifierWithCompletionHandler:^(
                        NSString* leaderboardIdentifier, NSError* error) {

                    if (error != nil) {
                        NSLog(@"%@", [error localizedDescription]);
                    } else {
                        GC_leaderboardIdentifier = leaderboardIdentifier;
                    }
                }];
            } else {
                NSLog(@"GameCenter is disabled by user: %@", [error localizedDescription]);
                GC_gameCenterEnabled = NO;
            }
        }
    };
}

namespace ek {
void game_services_init() {
    EK_DEBUG("game-services initialize");
    GC_authenticateLocalPlayer();
}

void leader_board_show(const char*) {
    if (GC_leaderboardIdentifier != nil) {
        GKGameCenterViewController* gcViewController = [[GKGameCenterViewController alloc] init];
        gcViewController.gameCenterDelegate = game_center_delegate;

        gcViewController.viewState = GKGameCenterViewControllerStateLeaderboards;
        gcViewController.leaderboardIdentifier = GC_leaderboardIdentifier;
        //    gcViewController.viewState = GKGameCenterViewControllerStateAchievements;

        [GC_rootViewController presentViewController:gcViewController animated:YES completion:nil];
    }
}

void leader_board_submit(const char*, int score) {
    [FIRAnalytics logEventWithName:kFIREventPostScore
                        parameters:@{kFIRParameterScore: @(score).stringValue}];

    if (GC_leaderboardIdentifier != nil) {
        GKScore* gkScore = [[GKScore alloc] initWithLeaderboardIdentifier:GC_leaderboardIdentifier];
        gkScore.value = score;

        [GKScore reportScores:@[gkScore] withCompletionHandler:^(NSError* error) {
            if (error != nil) {
                NSLog(@"%@", [error localizedDescription]);
            }
        }];
    }
}

void achievement_update(const char* id, int increment) {
    // TODO: incremental ID
    if (increment != 0) return;

    NSString* s_achievement_id = [NSString stringWithUTF8String:id];
    GKAchievement* ach = [[GKAchievement alloc] initWithIdentifier:s_achievement_id];
    ach.percentComplete = 100.0f;

    NSArray* achievements = @[ach];
    [GKAchievement reportAchievements:achievements withCompletionHandler:^(NSError* error) {
        if (error != nil) {
            NSLog(@"%@", [error localizedDescription]);
        }
    }];
}

void achievement_show() {
    if(!GC_gameCenterEnabled) {
        // TODO: handle disabled game center with alert pop-up
        return;
    }
    GKGameCenterViewController* gcViewController = [[GKGameCenterViewController alloc] init];
    gcViewController.gameCenterDelegate = game_center_delegate;
    gcViewController.viewState = GKGameCenterViewControllerStateAchievements;
    [GC_rootViewController presentViewController:gcViewController animated:YES completion:nil];
}
}
