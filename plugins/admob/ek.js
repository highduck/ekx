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
                pods: ["Google-Mobile-Ads-SDK"],
                plist: [
                    {
                        GADApplicationIdentifier: ctx.ios.admob_app_id,
                        SKAdNetworkItems: [
                            {SKAdNetworkIdentifier: "cstr6suwn9.skadnetwork"}
                        ]
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
        "implementation 'com.google.firebase:firebase-ads'"
    );

    ctx.build.android.add_manifest_application.push(`
        <meta-data android:name="com.google.android.gms.ads.APPLICATION_ID" android:value="@string/admob_app_id"/>`);

    ctx.build.android.xmlStrings.admob_app_id = ctx.android.admob_app_id;

    if (ctx.current_target === "android") {
        if (!ctx.android.admob_app_id) {
            console.error("please set android.admob_app_id");
        }
    }
};