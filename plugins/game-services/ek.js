/**
 *
 * @param {Project} project
 */
function setup(project) {
    if (project.current_target === "android") {
        if (!project.android.game_services_id) {
            console.error("please set android.game_services_id !!!");
        }
    }

    project.addModule({
        name: "plugin-game-services",
        path: __dirname,
        cpp: "src",
        android: {
            cpp: "android",
            android_java: "android/java",
            android_dependency: [
                "implementation 'com.google.android.gms:play-services-base:17.6.0'",
                "implementation 'com.google.android.gms:play-services-games:21.0.0'",
                "implementation 'com.google.android.gms:play-services-auth:19.2.0'"
            ],
            android_manifestApplication: [
                `<meta-data android:name="com.google.android.gms.games.APP_ID" android:value="@string/gs_app_id" />`,
                `<meta-data android:name="com.google.android.gms.version" android:value="@integer/google_play_services_version" />`
            ],
            android_strings: {
                gs_app_id: project.android.game_services_id
            }
        },
        ios: {
            cpp: "ios",
            xcode_capability: "com.apple.GameCenter",
            xcode_framework: "GameKit"
        },
        web: {
            cpp: "null"
        },
        macos: {
            cpp: "null"
        },
    });
}

module.exports = setup;