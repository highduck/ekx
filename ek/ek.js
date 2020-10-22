const path = require('path');

module.exports = (ctx) => {
    ctx.market_asset = "assets/res";
    ctx.modules = [
        {
            name: "app",
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
                    capabilities: ["com.apple.GameCenter"],
                    frameworks: [
                        "UIKit", "OpenGLES", "QuartzCore", "Foundation",
                        "OpenAL", "AudioToolbox", "AVFoundation",
                        "GameKit"
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
        }
    ];
    ctx.build = {
        android: {
            dependencies: [],
            add_manifest: [],
            add_manifest_application: [],
            source_dirs: [],
        }
    };
    ctx.ios = {};
    ctx.html = {};
};