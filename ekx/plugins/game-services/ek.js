const path = require("path");

module.exports = (ctx) => {
    ctx.addModule({
        name: "plugin-game-services",
        cpp: [path.join(__dirname, "src")],
        android: {
            cpp: [path.join(__dirname, "android")],
            java: [path.join(__dirname, "android/java")]
        },
        ios: {
            cpp: [path.join(__dirname, "ios")],
            xcode: {
                capabilities: ["com.apple.GameCenter"],
                frameworks: ["GameKit"]
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
        "implementation 'com.google.android.gms:play-services-base:17.6.0'",
        "implementation 'com.google.android.gms:play-services-games:21.0.0'",
        "implementation 'com.google.android.gms:play-services-auth:19.0.0'",
    );

    ctx.build.android.add_manifest_application.push(`
        <meta-data
            android:name="com.google.android.gms.games.APP_ID"
            android:value="@string/gs_app_id" />
        <meta-data
            android:name="com.google.android.gms.version"
            android:value="@integer/google_play_services_version" />`);

    if (ctx.current_target === "android") {
        if (ctx.android.game_services_id) {
            ctx.build.android.xmlStrings.gs_app_id = ctx.android.game_services_id;
        }
        else {
            console.error("please set android.game_services_id");
        }
    }
};