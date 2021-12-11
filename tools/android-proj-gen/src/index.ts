import {BaseModuleConfig, BuildTop, GradlePrinter} from "./gradle";
import * as fs from "fs";
import * as path from "path";
import {getAndroidSdkRoot, getJavaHome} from "./android";
import {AndroidManifest} from "./androidManifest";
import {writeColorsXML, writeStringsXML} from "./res";

export {openAndroidStudioProject} from "./android";

export class AndroidProjGen {
    name: string = "my-project";

    top: BuildTop = generateTopModule();
    app: BaseModuleConfig = generateNativeApp();
    androidManifest = new AndroidManifest();

    // "app/src/main/res/values/strings.xml"
    strings: { [key: string]: string } = {
        // app title
        app_name: "My App",
        // applicationId
        package_name: "com.eliasku.native_project_preset"
    };

    colors: { [name: string]: string } = {
        colorPrimary: "#008577",
        colorPrimaryDark: "#00574B",
        colorAccent: "#D81B60"
    };

    gradleProperties = new Properties({
        "org.gradle.jvmargs": "-Xmx2048m -Dfile.encoding=UTF-8",
        //"org.gradle.parallel":true,
        "android.useAndroidX": true,
        "android.enableJetifier": true,
        "org.gradle.java.home": getJavaHome(11) || undefined
    });


    localProperties = new Properties({
        "sdk.dir": getAndroidSdkRoot()
    });

    fastlane = true;
    wrapper = true;

    save(dir: string) {
        try {
            fs.rmSync(dir, {recursive: true});
        } catch {
        }
        try {
            fs.mkdirSync(dir, {recursive: true});
            fs.mkdirSync(path.join(dir, "app"));
        } catch {
        }

        //fs.cpSync(path.resolve(__dirname, "../project"), dir, {recursive: true});
        const srcDir = path.resolve(__dirname, "../project");
        const filesToCopy = [
            // top
            ".gitignore",

            /// app
            "app/.gitignore",
            "app/build.gradle",
            "app/multidex-config.pro",
            "app/proguard-rules.pro"
        ];
        for (const file of filesToCopy) {
            fs.copyFileSync(path.join(srcDir, file), path.join(dir, file));
        }

        if (this.fastlane) {
            try {
                fs.mkdirSync(path.join(dir, "fastlane"));
            } catch {
            }
            const filesToCopy = [
                "Gemfile",
                "fastlane/Appfile",
                "fastlane/Fastfile"
            ];
            for (const file of filesToCopy) {
                fs.copyFileSync(path.join(srcDir, file), path.join(dir, file));
            }
        }

        if (this.wrapper) {
            try {
                fs.mkdirSync(path.join(dir, "gradle/wrapper"), {recursive: true});
            } catch {
            }
            const filesToCopy = [
                "gradlew",
                "gradlew.bat",
                "gradle/wrapper/gradle-wrapper.jar",
                "gradle/wrapper/gradle-wrapper.properties",
            ];
            for (const file of filesToCopy) {
                fs.copyFileSync(path.join(srcDir, file), path.join(dir, file));
            }
        }

        {
            const p = new GradlePrinter();
            if (this.top.buildscript) {
                p.block("buildscript", () => {
                    p.printBaseModuleBody(this.top.buildscript);
                });
            }
            p.block("task clean(type: Delete)", () => {
                p.writeln("delete rootProject.buildDir");
            });
            p.save(path.join(dir, "build.gradle"));
        }

        {
            const p = new GradlePrinter();
            p.block("dependencyResolutionManagement", () => {
                p.writeln(`repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)`);
                p.printRepositorySection(this.top.allprojects.repositories);
            });
            p.writeln(`rootProject.name = "${this.name}"`);
            p.writeln("include ':app'");
            p.save(path.join(dir, "settings.gradle"));
        }

        this.gradleProperties.save(path.join(dir, "gradle.properties"));
        this.localProperties.save(path.join(dir, "local.properties"));

        {
            try {
                fs.mkdirSync(path.join(dir, "app/src/main/res/values"), {recursive: true});
            } catch {
            }

            writeStringsXML(path.join(dir, "app/src/main/res/values/strings.xml"), this.strings);
            writeColorsXML(path.join(dir, "app/src/main/res/values/colors.xml"), this.colors);

            const filesToCopy = [
                //"app/src/main/res/values/colors.xml",
                //"app/src/main/res/values/strings.xml",
                "app/src/main/res/values/styles.xml"
            ];
            for (const file of filesToCopy) {
                fs.copyFileSync(path.join(srcDir, file), path.join(dir, file));
            }
            const p = new GradlePrinter();
            p.printBaseModuleBody(this.app);
            p.save(path.join(dir, "app/build.gradle"))

            this.androidManifest.save(path.join(dir, "app/src/main/AndroidManifest.xml"));
        }
    }
}

export function generateTopModule(): BuildTop {
    return {
        buildscript: {
            repositories: [{name: "google"}, {name: "mavenCentral"}],
            dependencies: [{
                type: "classpath",
                name: "com.android.tools.build:gradle",
                version: "7.0.2"
            }]
        },
        allprojects: {
            repositories: [{name: "google"}, {name: "mavenCentral"}],
        }
    };
}

export function generateNativeApp(): BaseModuleConfig {
    return {
        plugins: [
            "com.android.application"
        ],
        android: {
            compileSdk: 30,
            buildToolsVersion: "30.0.3",
            ndkVersion: "23.0.7599858",
            signingConfigs: {},
            defaultConfig: {
                applicationId: "com.eliasku.native_project_preset",
                versionCode: 1,
                versionName: "1.0",
                minSdk: 19,
                targetSdk: 30,
                multiDexEnabled: true,
                testInstrumentationRunner: "androidx.test.runner.AndroidJUnitRunner",
                externalNativeBuild: {
                    cmake: {
                        cppFlags: "-std=c++17"
                    }
                }
            },
            buildTypes: {
                release: {
                    minifyEnabled: true,
                    proguardFiles: [`getDefaultProguardFile('proguard-android-optimize.txt')`, `file('proguard-rules.pro')`],
                    multiDexKeepProguard: [`file('multidex-config.pro')`],
                    ndk: {
                        debugSymbolLevel: "SYMBOL_TABLE"
                    }
                }
            },
            sourceSets: {
                main: {
                    java: {
                        srcDirs: []
                    },
                    aidl: {
                        srcDirs: []
                    },
                    assets: {
                        srcDirs: []
                    }
                }
            },
            compileJavaVersion: "1.8",
            externalNativeBuild: {
                cmake: {
                    path: "CMakeLists.txt",
                    version: "3.19.0+"
                }
            },
            bundle: {
                splitLanguage: false,
                splitDensity: false,
                splitAbi: true
            }
        },
        dependencies: [
            `implementation fileTree(dir: 'libs', include: ['*.jar'])`,
            {type: "implementation", name: "androidx.annotation:annotation", version: "1.2.0"},
            {type: "implementation", name: "androidx.multidex:multidex", version: "2.0.1"},
            {type: "implementation", name: "com.getkeepsafe.relinker:relinker", version: "1.4.4"}
        ]
    };
}

class Properties {

    constructor(
        readonly dict: { [key: string]: any } = {}
    ) {

    }

    toString(): string {
        let buf = "";
        for (const key of Object.keys(this.dict)) {
            buf += `${key}=${this.dict[key]}\n`;
        }
        return buf;
    }

    save(filepath: string) {
        fs.writeFileSync(filepath, this.toString(), "utf8");
    }
}

