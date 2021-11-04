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
                "android",
                "GLESv2"
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
            ]
        },
        ios: {
            xcode_framework: [
                "Foundation",
                "UIKit",
                "Metal",
                "MetalKit",
                "QuartzCore",
            ],
            xcode_pod: [
                "Firebase/Crashlytics",
                "Firebase/Analytics"
            ]
        },
        web: {
            cpp: "platforms/web",
            js: "platforms/web",
            cpp_lib: ["GLESv2", "GL"]
        },
        windows: {
            cpp: "platforms/windows"
        },
        linux: {
            cpp: "platforms/linux"
        }
    });
}

module.exports = setup;