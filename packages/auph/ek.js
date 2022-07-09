/**
 *
 * @param {Project} project
 */
export async function setup(project) {
    project.addModule({
        name: "auph",
        cpp_include: "include",
        android: {
            cpp_include: [
                "android/oboe/src",
                "android/oboe/include"
            ],
            cpp_lib: ["android", "log", "OpenSLES"],
            android_java: "android/java",
            android_permission: "android.permission.VIBRATE"
        },
        apple: {
            xcode_framework: ["Foundation", "AudioToolbox"]
        },
        macos: {
            xcode_framework: "CoreAudio"
        },
        ios: {
            xcode_framework: "AVFoundation"
        },
        web: {
            js_pre: "web/dist/emscripten"
        },
        windows: {},
        linux: {}
    });

    await project.importModule("@ekx/stb");
    await project.importModule("@ekx/dr-libs");
}
