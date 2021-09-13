/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.addModule({
        name: "ek-app",
        path: __dirname,
        cpp: "src",
        android: {
            cpp: "platforms/android",
            cpp_lib: [
                // basic Android libraries
                "log",
                "android"
            ],
            android_java: "platforms/android/java"
        },
        apple: {
            cpp: "platforms/apple"
        },
        macos: {
            xcode_framework: [
                "Cocoa",
                "Metal",
                "MetalKit",
                "QuartzCore",
                "AudioToolbox",
            ]
        },
        ios: {
            xcode_framework: [
                "Foundation",
                "UIKit",
                "Metal",
                "MetalKit",
                "QuartzCore",
                "AudioToolbox"
            ],
            xcode_pod: [
                "Firebase/Crashlytics",
                "Firebase/Analytics"
            ]
        },
        web: {
            cpp: "platforms/web",
            js: "platforms/web"
        },
        windows: {
            cpp: "platforms/windows"
        },
        linux: {
            cpp: "platforms/linux"
        }
    });
    project.importModule("@ekx/ekx/core", __dirname);
}

module.exports = setup;