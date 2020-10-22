const path = require("path");

module.exports = (ctx) => {
    ctx.modules.push({
        name: "mini-ads",
        cpp: [path.join(__dirname, "src")],
        android: {
            cpp: [path.join(__dirname, "android")],
            java: [path.join(__dirname, "android/java")],
            aidl: [path.join(__dirname, "android/aidl")]
        },
        ios: {
            cpp: [path.join(__dirname, "ios")],
            xcode: {
                capabilities: ["com.apple.InAppPurchase"],
                frameworks: ["StoreKit"],
                pods: ["Firebase/AdMob"],
                plist: [
                    {
                        GADApplicationIdentifier: ctx.ios.admob_app_id
                    }
                ]
            }
        }
    });

    ctx.build.android.dependencies.push(
        "implementation 'com.google.firebase:firebase-ads'"
    );

    console.log(ctx.build.android.add_manifest);
    ctx.build.android.add_manifest.push(`
    <uses-permission android:name="com.android.vending.BILLING" />`);

    ctx.build.android.add_manifest_application.push(`
        <meta-data android:name="com.google.android.gms.ads.APPLICATION_ID" android:value="@string/admob_app_id"/>`);

    if (ctx.current_target === "android") {
        if (!ctx.android.admob_app_id) {
            console.error("please set android.admob_app_id");
        }
    }
};