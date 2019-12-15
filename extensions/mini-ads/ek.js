const path = require("path");

module.exports = class File {
    constructor(ctx) {
        // === ANDROID ===

        ctx.modules.push({
            name: "mini-ads",
            cpp: [path.join(__dirname, "src")],
            android: {
                cpp: [path.join(__dirname, "android")],
                java: [path.join(__dirname, "android/java")]
            },
            ios: {
                cpp: [path.join(__dirname, "ios")],
                xcode: {
                    capabilities: ["com.apple.InAppPurchase"],
                    frameworks: ["StoreKit"],
                    pods: ["Firebase/AdMob"]
                }
            }
        });
        
        // ctx.build.android.java_src.push(
        //     path.join(__dirname, "android/java")
        // );

        ctx.build.android.dependencies.push(
            "implementation 'com.google.firebase:firebase-ads:18.3.0'",
            "implementation 'com.android.billingclient:billing:1.1'"
        );

        console.log(ctx.build.android.add_manifest);
        ctx.build.android.add_manifest.push(`
    <meta-data android:name="ADMOB_ALLOW_LOCATION_FOR_ADS" android:value="true" />
    <uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
    <uses-permission android:name="com.android.vending.BILLING" />`);

        ctx.build.android.add_manifest_application.push(`
        <meta-data android:name="com.google.android.gms.ads.APPLICATION_ID" android:value="@string/admob_app_id"/>`);

        // === IOS ===
        // TODO: Info.plist GADApplicationIdentifier
        // TODO: dict["GADIsAdManagerApp"] = true; was there???
        ctx.build.ios.billing = true;

        if (ctx.current_target === "android") {
            if (!ctx.android.admob_app_id) {
                console.error("please set android.admob_app_id");
            }
        }
    }
};