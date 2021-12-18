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
            const configPath = path.join(path.resolve(project.projectPath, project.android.googleServicesConfigDir), configFile);
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
            const configPath = path.join(path.resolve(project.projectPath, project.android.googleServicesConfigDir), configFile);
            try {
                // CWD is project generated path here
                fs.copyFileSync(configPath, configFile)
            } catch (err) {
                console.error("missing google-service config", configPath);
            }
        } else if (project.current_target === "web") {
            if (!project.web.firebaseConfig) {
                console.error("please set `web.firebaseConfig` !!!");
            }
            project.web.headCode.push(`<script type="module">
  import { initializeApp } from "https://www.gstatic.com/firebasejs/9.1.2/firebase-app.js";
  import { getAnalytics } from "https://www.gstatic.com/firebasejs/9.1.2/firebase-analytics.js";
  var firebaseConfig = ${JSON.stringify(project.web.firebaseConfig)};
  var app = initializeApp(firebaseConfig);
  var analytics = getAnalytics(app);
</script>`);
        }
    });

    project.addModule({
        name: "plugin-firebase",
        path: __dirname,
        cpp: "src",
        android: {
            // TODO: fastlane
            // gradle(task: "uploadCrashlyticsSymbolFile", build_type: "Release")

            android_java: "java",
            // Import the BoM for the Firebase platform
            // Check Release Notes for updates: https://firebase.google.com/support/release-notes/android
            android_buildScriptDependency: [
                `classpath 'com.google.gms:google-services:4.3.10'`,
                `classpath 'com.google.firebase:firebase-crashlytics-gradle:2.8.1'`
            ],
            android_gradleApplyPlugin: ['com.google.gms.google-services', 'com.google.firebase.crashlytics'],
            android_gradleConfigRelease: `firebaseCrashlytics {
                nativeSymbolUploadEnabled true
                symbolGenerator {
                    breakpad()
                }
            }`,
            android_dependency: [
                `implementation platform('com.google.firebase:firebase-bom:28.4.2')`,
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
                    "src/ek/firebase.c"
                ],
                flags: "-x objective-c"
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
        },
        web: {
         //   js: "web/dist"
        }
    });

    project.importModule("@ekx/app", __dirname);
}

module.exports = setup;