module.exports = class File {
    constructor(ctx) {
        // === ANDROID ===

        ctx.build.android.java_src.push(
            path.join(__dirname, "android/java")
        );

        ctx.build.android.dependencies.push(
            "implementation 'com.google.firebase:firebase-ads:18.3.0'",
            "implementation 'com.android.billingclient:billing:1.1'"
        );

        ctx.build.android.add_manifest.push(```
    <meta-data android:name="ADMOB_ALLOW_LOCATION_FOR_ADS" android:value="true" />
    <uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
    <uses-permission android:name="com.android.vending.BILLING" />```);

        ctx.build.android.add_manifest_application.push(```
        <meta-data android:name="com.google.android.gms.ads.APPLICATION_ID" android:value="@string/admob_app_id"/>```);

        // === IOS ===
        // TODO: Info.plist GADApplicationIdentifier
        // TODO: dict["GADIsAdManagerApp"] = true; was there???
        ctx.build.ios.billing = true;
        ctx.build.ios.dependencies.push("pod 'Firebase/AdMob'");

        if (ctx.current_target === "android") {
            if (!ctx.android.admob_app_id) {
                console.error("please set android.admob_app_id");
            }
        }
    }
};