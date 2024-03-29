import {Project} from "../../modules/cli/project.js";
import {logger} from "../../modules/cli/logger.js";

export async function setup(project: Project) {
    if (project.current_target === "android") {
        if (!project.android.game_services_id) {
            logger.error("please set android.game_services_id !!!");
        }
    }

    project.addModule({
        name: "plugin-game-services",
        cpp_include: "include",
        cpp: "src",
        android: {
            android_java: "java",
            android_dependency: [
                "implementation 'com.google.android.gms:play-services-base:18.2.0'",
                "implementation 'com.google.android.gms:play-services-games:23.1.0'",
                "implementation 'com.google.android.gms:play-services-auth:20.4.1'"
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
            cpp_flags: {
                files: [
                    "src/ek_game_services.c"
                ],
                flags: "-x objective-c"
            },
            xcode_capability: "com.apple.GameCenter",
            xcode_framework: "GameKit"
        }
    });

    await project.importModule("../../packages/app/ek.ts");
}
