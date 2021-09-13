const path = require("path");
const fs = require("fs");

/**
 *
 * @param {Project} project
 */
function setup(project) {
    project.onProjectGenerated.push(() => {
        if (project.current_target === "android") {
            if (!project.android.googleServicesConfigDir) {
                console.error("please set `android.googleServicesConfigDir` !!!");
            }
            const configFile = "google-services.json";
            const configPath = path.join(path.resolve(project.path.CURRENT_PROJECT_DIR, project.android.googleServicesConfigDir), configFile);
            try {
                // CWD is project generated path here
                fs.copyFileSync(configPath, path.join("app", configFile))
            } catch (err) {
                console.error("missing google-service config", configPath);
            }
        } else if (project.current_target === "ios") {
            if (!project.ios.googleServicesConfigDir) {
                console.error("please set `ios.googleServicesConfigDir` !!!");
            }
            const configFile = "GoogleService-Info.plist";
            const configPath = path.join(path.resolve(project.path.CURRENT_PROJECT_DIR, project.android.googleServicesConfigDir), configFile);
            try {
                // CWD is project generated path here
                fs.copyFileSync(configPath, configFile)
            } catch (err) {
                console.error("missing google-service config", configPath);
            }
        }
    });

    project.addModule({
        name: "plugin-firebase",
        path: __dirname,
        cpp: "src",
        android: {
            // TODO: fastlane
            // gradle(task: "uploadCrashlyticsSymbolFile", build_type: "Release")

            android_java: "android/java",
            // Import the BoM for the Firebase platform
            // Check Release Notes for updates: https://firebase.google.com/support/release-notes/android
            android_buildScriptDependency: `classpath 'com.google.firebase:firebase-crashlytics-gradle:2.7.1'`,
            android_gradleApplyPlugin: 'com.google.firebase.crashlytics',
            android_gradleConfigRelease: `firebaseCrashlytics {
                nativeSymbolUploadEnabled true
                symbolGenerator {
                    breakpad()
                }
            }`,
            android_dependency: [
                `implementation platform('com.google.firebase:firebase-bom:28.4.0')`,
                `implementation 'com.google.firebase:firebase-crashlytics-ndk'`,
                `implementation 'com.google.firebase:firebase-analytics'`,
            ]
        },
        ios: {
            xcode_pod: [
                "Firebase/Crashlytics",
                "Firebase/Analytics"
            ],
            cpp_flags: {
                files: [
                    "src/ek/firebase/Firebase.cpp"
                ],
                flags: "-x objective-c++"
            },
            xcode_file: [
                "GoogleService-Info.plist"
            ],
            xcode_projectPythonPostScript: `
# Fabric / Crashlytics
shell = PBXShellScriptBuildPhase.create(
    "\${PODS_ROOT}/FirebaseCrashlytics/run",
    input_paths=["$(SRCROOT)/$(BUILT_PRODUCTS_DIR)/$(INFOPLIST_PATH)"]
)
project.objects[shell.get_id()] = shell
app_target.add_build_phase(shell)`
        }
    });
}

module.exports = setup;