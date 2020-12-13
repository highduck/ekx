const path = require('path');

module.exports = (ctx) => {
    ctx.addModule({
        name: "ek",
        cpp: [path.join(__dirname, "src")],
        android: {
            cpp: [path.join(__dirname, "platforms/android")],
            java: [path.join(__dirname, "platforms/android/java")]
        },
        macos: {
            cpp: [
                path.join(__dirname, "platforms/apple"),
                path.join(__dirname, "platforms/mac")
            ]
        },
        ios: {
            cpp: [
                path.join(__dirname, "platforms/apple"),
                path.join(__dirname, "platforms/ios")
            ],
            cpp_flags: {
                files: [
                    path.join(__dirname, "src/raudio/raudio.cpp")
                ],
                flags: "-x objective-c++"
            },
            xcode: {
                frameworks: [
                    "UIKit", "OpenGLES", "QuartzCore", "Foundation",
                    "OpenAL", "AudioToolbox", "AVFoundation"
                ],
                pods: [
                    "Firebase/Crashlytics",
                    "Firebase/Analytics"
                ]
            }
        },
        web: {
            cpp: [path.join(__dirname, "platforms/web")]
        },
        windows: {
            cpp: [path.join(__dirname, "platforms/windows")]
        },
        linux: {
            cpp: [path.join(__dirname, "platforms/linux")]
        }
    });

};