const path = require("path");

module.exports = (ctx) => {
    ctx.addModule({
        name: "plugin-admob",
        cpp: [path.join(__dirname, "src")],
        android: {
            cpp: [path.join(__dirname, "android")],
            java: [path.join(__dirname, "android/java")]
        },
        ios: {
            cpp: [path.join(__dirname, "ios")],
            xcode: {
                //frameworks: ["AppTrackingTransparency"],
                pods: ["Google-Mobile-Ads-SDK"],
                plist: [
                    {
                        NSUserTrackingUsageDescription: "This identifier will be used to deliver personalized ads to you.",
                        //GADDelayAppMeasurementInit: true,
                        GADApplicationIdentifier: ctx.ios.admob_app_id,
                        SKAdNetworkItems: [
                            {SKAdNetworkIdentifier: "cstr6suwn9.skadnetwork"},
                            // list of third-party ad buyers: https://developers.google.com/admob/ios/3p-skadnetworks
                            {SKAdNetworkIdentifier: "4fzdc2evr5.skadnetwork"},
                            {SKAdNetworkIdentifier: "2fnua5tdw4.skadnetwork"},
                            {SKAdNetworkIdentifier: "ydx93a7ass.skadnetwork"},
                            {SKAdNetworkIdentifier: "5a6flpkh64.skadnetwork"},
                            {SKAdNetworkIdentifier: "p78axxw29g.skadnetwork"},
                            {SKAdNetworkIdentifier: "v72qych5uu.skadnetwork"},
                            {SKAdNetworkIdentifier: "c6k4g5qg8m.skadnetwork"},
                            {SKAdNetworkIdentifier: "s39g8k73mm.skadnetwork"},
                            {SKAdNetworkIdentifier: "3qy4746246.skadnetwork"},
                            {SKAdNetworkIdentifier: "3sh42y64q3.skadnetwork"},
                            {SKAdNetworkIdentifier: "f38h382jlk.skadnetwork"},
                            {SKAdNetworkIdentifier: "hs6bdukanm.skadnetwork"},
                            {SKAdNetworkIdentifier: "prcb7njmu6.skadnetwork"},
                            {SKAdNetworkIdentifier: "wzmmz9fp6w.skadnetwork"},
                            {SKAdNetworkIdentifier: "yclnxrl5pm.skadnetwork"},
                            {SKAdNetworkIdentifier: "4468km3ulz.skadnetwork"},
                            {SKAdNetworkIdentifier: "t38b2kh725.skadnetwork"},
                            {SKAdNetworkIdentifier: "7ug5zh24hu.skadnetwork"},
                            {SKAdNetworkIdentifier: "9rd848q2bz.skadnetwork"},
                            {SKAdNetworkIdentifier: "n6fk4nfna4.skadnetwork"},
                            {SKAdNetworkIdentifier: "kbd757ywx3.skadnetwork"},
                            {SKAdNetworkIdentifier: "9t245vhmpl.skadnetwork"},
                            {SKAdNetworkIdentifier: "2u9pt9hc89.skadnetwork"},
                            {SKAdNetworkIdentifier: "8s468mfl3y.skadnetwork"},
                            {SKAdNetworkIdentifier: "av6w8kgt66.skadnetwork"},
                            {SKAdNetworkIdentifier: "klf5c3l5u5.skadnetwork"},
                            {SKAdNetworkIdentifier: "ppxm28t8ap.skadnetwork"},
                            {SKAdNetworkIdentifier: "424m5254lk.skadnetwork"},
                            {SKAdNetworkIdentifier: "uw77j35x4d.skadnetwork"},
                            {SKAdNetworkIdentifier: "e5fvkxwrpn.skadnetwork"},
                            {SKAdNetworkIdentifier: "zq492l623r.skadnetwork"},
                            {SKAdNetworkIdentifier: "3qcr597p9d.skadnetwork"},
                        ],
                    }
                ]
            }
        },
        web: {
            cpp: [path.join(__dirname, "null")]
        },
        macos: {
            cpp: [path.join(__dirname, "null")]
        },
    });

    ctx.build.android.dependencies.push(
        `implementation 'com.google.firebase:firebase-ads'`,
        //`implementation 'com.google.android.ump:user-messaging-platform:1.0.0'`,
    );

    ctx.build.android.add_manifest_application.push(`
        <meta-data android:name="com.google.android.gms.ads.APPLICATION_ID" android:value="@string/admob_app_id"/>
        <!--<meta-data android:name="com.google.android.gms.ads.DELAY_APP_MEASUREMENT_INIT" android:value="true"/>-->
`);

    ctx.build.android.xmlStrings.admob_app_id = ctx.android.admob_app_id;

    if (ctx.current_target === "android") {
        if (!ctx.android.admob_app_id) {
            console.error("please set android.admob_app_id");
        }
    }
};