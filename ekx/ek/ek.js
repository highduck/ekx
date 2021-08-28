/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "ek",
        path: __dirname,
        cpp: ["src"],
        android: {
            cpp: "platforms/android",
            cpp_lib: [
                // basic Android libraries
                "log",
                "android",
                // for graphics module (sokol_gfx)
                "GLESv2"
            ],
            android_java: "platforms/android/java"
        },
        apple: {
            cpp: "platforms/apple",
            cpp_flags: {
                files: [
                    "src/ek/graphics/graphics.cpp",
                    "src/ek/audio/audio.cpp"
                ],
                flags: "-x objective-c++"
            },
        },
        ios: {
            xcode_framework: [
                "Foundation", "UIKit", "Metal", "MetalKit", "QuartzCore"
            ],
            xcode_pod: [
                "Firebase/Crashlytics",
                "Firebase/Analytics"
            ]
        },
        web: {
            cpp: "platforms/web",
            cpp_lib: "GLESv2",
            js: "platforms/web"
        },
        windows: {
            cpp: "platforms/windows"
        },
        linux: {
            cpp: "platforms/linux"
        }
    });
    project.importModule("auph", __dirname);
}

module.exports = setup;