const path = require("path");

module.exports = class File {
    constructor(ctx) {
        ctx.modules.push({
            name: "audiomini",
            cpp: [path.join(__dirname, "src")],
            android: {
                cpp: [path.join(__dirname, "android")],
                java: [path.join(__dirname, "android/java")]
            },
            ios: {
                cpp: [
                    path.join(__dirname, "apple"),
                    path.join(__dirname, "ios")
                ],
                cpp_flags: {
                    files: [
                        path.join(__dirname, "ios/SimpleAudioEngine_objc.mm"),
                        path.join(__dirname, "ios/CocosDenshion.mm"),
                        path.join(__dirname, "ios/CDOpenALSupport.mm"),
                        path.join(__dirname, "ios/CDAudioManager.mm"),
                    ],
                    flags: "-fno-objc-arc"
                },
                xcode: {
                    capabilities: ["com.apple.InAppPurchase"],
                    frameworks: ["StoreKit"],
                    pods: ["Firebase/AdMob"],
                    plist: [
                        {
                            GADIsAdManagerApp: true,
                            GADApplicationIdentifier: ctx.ios.admob_app_id
                        }
                    ]
                }
            }
        });
    }
};