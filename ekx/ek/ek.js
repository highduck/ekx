const path = require('path');

module.exports = (project) => {
    project.addModule({
        name: "ek",
        path: __dirname,
        cpp: [path.join(__dirname, "src")],
        android: {
            cpp: [path.join(__dirname, "platforms/android")],
            java: [path.join(__dirname, "platforms/android/java")]
        },
        macos: {
            cpp: [
                path.join(__dirname, "platforms/apple"),
                path.join(__dirname, "platforms/mac")
            ],
            cpp_flags: {
                files: [
                    path.join(__dirname, "src/ek/graphics/graphics.cpp"),
                    path.join(__dirname, "src/ek/audio/audio.cpp")
                ],
                flags: "-x objective-c++"
            },
        },
        ios: {
            cpp: [
                path.join(__dirname, "platforms/apple"),
                path.join(__dirname, "platforms/ios")
            ],
            cpp_flags: {
                files: [
                    path.join(__dirname, "src/ek/graphics/graphics.cpp"),
                    path.join(__dirname, "src/ek/audio/audio.cpp")
                ],
                flags: "-x objective-c++"
            },
            xcode: {
                frameworks: [
                    "Foundation", "UIKit", "Metal", "MetalKit",
                    "QuartzCore"
                ],
                pods: [
                    "Firebase/Crashlytics",
                    "Firebase/Analytics"
                ]
            }
        },
        web: {
            cpp: [path.join(__dirname, "platforms/web")],
            js: [path.join(__dirname, "platforms/web")]
        },
        windows: {
            cpp: [path.join(__dirname, "platforms/windows")]
        },
        linux: {
            cpp: [path.join(__dirname, "platforms/linux")]
        }
    });
    project.importModule("auph", __dirname);
};