import {
    copyFile,
    copyFolderRecursiveSync,
    deleteFolderRecursive,
    execute,
    isDir,
    isFile,
    makeDirs,
    readText,
    replaceAll,
    replaceInFile,
    writeText
} from "../utils";
import {XmlDocument} from 'xmldoc';
import * as path from "path";
import {buildAssetsAsync} from "../assets";
import {collectObjects, collectSourceFiles, collectSourceRootsAll, collectStrings} from "../collectSources";
import {copySigningKeys, printSigningConfigs} from "./signing";
import {execSync} from "child_process";
import {androidBuildAppIconAsync} from "./androidAppIcon";
import * as fs from "fs";
import {writeFileSync} from "fs";
import {Project} from "../project";
import {CMakeGenerateProject, CMakeGenerateTarget, cmakeLists} from "../cmake/generate";
import {logger} from "../logger";

const platforms = ["android"];

function getAndroidSdkRoot() {
    return process.env.ANDROID_SDK_ROOT ?? path.join(process.env.HOME, 'Library/Android/sdk');
}

function getJavaHome() {
    // -v 1.8 ?
    return execSync('/usr/libexec/java_home -v 1.8', {
        encoding: 'utf-8'
    });
}

function gradle(...args: string[]) {
    // const ANDROID_SDK_ROOT = getAndroidSdkRoot();
    //
    // let env = Object.create(process.env);
    // env = Object.assign(env, {
    //     ANDROID_SDK_ROOT,
    //     //JAVA_HOME
    // });

    execSync(`./gradlew ${args.join(' ')}`, {
        stdio: 'inherit',
        encoding: 'utf-8',
        // env
    });
}

function open_android_project(android_project_path) {
    execute("open", ["-a", "/Applications/Android Studio.app", android_project_path]);
}

function copy_google_services_config_android(dir: string) {
    const config_file = "google-services.json";
    const config_path = path.join(dir, config_file);
    if (isFile(config_path)) {
        copyFile(config_path, path.join("app", config_file));
    } else {
        logger.warn("missing google-services.json", config_file);
    }
}

function mod_main_class(app_package_java) {
    const template_main_activity_java =
        "app/src/main/java/com/eliasku/template_android/MainActivity.java";
    const java_package_path = replaceAll(app_package_java, ".", "/");

    let text = readText(template_main_activity_java);
    text = replaceAll(text,
        "package com.eliasku.template_android;",
        `package ${app_package_java};`
    );
    deleteFolderRecursive("app/src/main/java/com");
    const main_activity_path = path.join("app/src/main/java", java_package_path);
    makeDirs(main_activity_path);
    writeText(path.join(main_activity_path, "MainActivity.java"), text);
}

function mod_android_manifest(ctx) {
    const android_manifest = collectStrings(ctx, "android_manifest", ["android"], false);
    const android_manifestApplication = collectStrings(ctx, "android_manifestApplication", ["android"], false);

    let orientation = "sensorPortrait";
    if (ctx.orientation === "landscape") {
        orientation = "sensorLandscape";
    } else if (ctx.orientation === "portrait") {
        orientation = "sensorPortrait";
    } else {
        logger.warn("unknown orientation", ctx.orientation);
    }
    const configChanges = "keyboardHidden|keyboard|orientation|screenSize|layoutDirection|locale|uiMode|screenLayout|smallestScreenSize|navigation";

    replaceInFile("app/src/main/AndroidManifest.xml", {
        'android:configChanges="PLACEHOLDER"': `android:configChanges="${configChanges}"`,
        "com.eliasku.template_android": ctx.android.package_id,
        'screenOrientation="sensorPortrait"': `screenOrientation="${orientation}"`,
        "<!-- TEMPLATE ROOT -->": android_manifest.join("\n"),
        "<!-- TEMPLATE APPLICATION -->": android_manifestApplication.join("\n")
    });
}

function createStringsXML(ctx) {
    const contents: any = {};
    const android_strings = collectObjects(ctx, "android_strings", platforms);
    for (const strings of android_strings) {
        for (const key of Object.keys(strings)) {
            contents[key] = strings[key];
        }
    }
    contents.app_name = ctx.title;
    contents.package_name = ctx.android.application_id;

    const doc = new XmlDocument(`<resources></resources>`);
    for (let key of Object.keys(contents)) {
        const val = contents[key];
        doc.children.push(new XmlDocument(`<string name="${key}" translatable="false">${val}</string>`));
    }
    writeText("app/src/main/res/values/strings.xml", doc.toString());
}

function mod_cmake_lists(ctx) {
    const cppSources = [];
    const cppExtensions = ["hpp", "hxx", "h", "cpp", "cxx", "c"];

    const cppRoots = collectSourceRootsAll(ctx, "cpp", platforms, ".");
    for (const cpp_dir of cppRoots) {
        collectSourceFiles(cpp_dir, cppExtensions, cppSources);
    }

    const cpp_include = collectSourceRootsAll(ctx, "cpp_include", platforms, ".");
    const cpp_define = collectStrings(ctx, "cpp_define", platforms, false);
    const cpp_lib = collectStrings(ctx, "cpp_lib", platforms, false);

    const cmakeName = "native-lib";
    const cmakeTarget: CMakeGenerateTarget = {
        type: "library",
        libraryType: "shared",
        name: cmakeName,
        sources: cppSources,
        includeDirectories: cppRoots.concat(cpp_include),
        linkLibraries: cpp_lib,
        linkOptions: ["-g"],
        compileOptions: [
            "-g",

            // "-ffunction-sections", //+
            // "-fdata-sections", //+
            "-fvisibility=hidden",//+

            // "-fvisibility-inlines-hidden",
            // "-ftree-vectorize",
            // "-ffor-scope",
            // "-pipe",

            "-ffast-math",
            "-fno-exceptions",
            "-fno-rtti",

            "-Wall",
            "-Wextra",

            //"-Werror",
            "-Wnon-virtual-dtor",
            "-Wsign-promo",
            //"-Wstrict-null-sentinel"
        ],
        compileDefinitions: cpp_define
    };

    // -fno-exceptions
    // -fno-rtti
    // -ffunction-sections
    // -fdata-sections
    // -fvisibility=hidden

    const cmakeProject: CMakeGenerateProject = {
        cmakeVersion: "3.19",
        project: cmakeName,
        targets: [cmakeTarget],
        compileOptions: [],
        compileDefinitions: []
    };

    //cmakeTarget.linkOptions.push("$<$<CONFIG:Release>:LINKER:--gc-sections>");
    cmakeTarget.linkOptions.push("-Wl,--build-id");
    //cmakeTarget.linkOptions.push("-Wl,--gc-sections")

    // cmakeTarget.compileDefinitions.push("$<$<NOT:$<CONFIG:Debug>>:NDEBUG>");
    // cmakeTarget.compileOptions.push("$<$<NOT:$<CONFIG:Debug>>:-Oz>");
    // cmakeTarget.linkOptions.push("$<$<NOT:$<CONFIG:Debug>>:-Oz>");

    fs.writeFileSync("CMakeLists.txt", cmakeLists(cmakeProject), "utf8");
}

export async function export_android(ctx: Project): Promise<void> {

    await Promise.all([
        buildAssetsAsync(ctx),
        androidBuildAppIconAsync(ctx, "export/android/res")
    ]);

    const platform_target = ctx.current_target; // "android"
    const platform_proj_name = ctx.name + "-" + ctx.current_target;
    const dest_dir = path.resolve(process.cwd(), "export");
    const dest_path = path.join(dest_dir, platform_proj_name);

    if (isDir(dest_path)) {
        logger.info("Remove old project", dest_path);
        deleteFolderRecursive(dest_path);
        logger.assert(!isDir(dest_path));
    } else {

    }

    // resolve absolute path to configs
    let googleServicesConfigDir = ctx.android.googleServicesConfigDir;
    let signingConfigPath = ctx.android.signingConfigPath;
    let serviceAccountKey = ctx.android.serviceAccountKey;
    if (googleServicesConfigDir) {
        googleServicesConfigDir = path.resolve(ctx.path.CURRENT_PROJECT_DIR, googleServicesConfigDir);
    }
    if (signingConfigPath) {
        signingConfigPath = path.resolve(ctx.path.CURRENT_PROJECT_DIR, signingConfigPath);
    }
    if (serviceAccountKey) {
        serviceAccountKey = path.resolve(ctx.path.CURRENT_PROJECT_DIR, serviceAccountKey);
    }

    copyFolderRecursiveSync(path.join(ctx.path.templates, `template-${platform_target}`), dest_path);
    const base_path = "../..";

    const cwd = process.cwd();
    process.chdir(dest_path);
    {
        const assets = collectSourceRootsAll(ctx, "assets", platforms, "app");
        const android_java = collectSourceRootsAll(ctx, "android_java", platforms, "app");
        const android_aidl = collectSourceRootsAll(ctx, "android_aidl", platforms, "app");
        const android_dependency = collectStrings(ctx, "android_dependency", platforms, false);

        function getGradleStringArrayExpr(arr: string[]): string {
            return "[" + arr.map(p => `'${p}'`).join(", ") + "]";
        }

        const source_sets = [
            `main.java.srcDirs += ${getGradleStringArrayExpr(android_java)}`,
            `main.aidl.srcDirs += ${getGradleStringArrayExpr(android_aidl)}`,
            `main.assets.srcDirs += ${getGradleStringArrayExpr(assets)}`
        ];

        let signingConfig = {};
        let signingConfigBasePath = "";

        if (signingConfigPath) {
            const signingConfigJson = fs.readFileSync(signingConfigPath, "utf-8");
            signingConfig = JSON.parse(signingConfigJson);
            signingConfigBasePath = path.dirname(signingConfigPath);
            copySigningKeys(signingConfig, signingConfigBasePath);
        } else {
            logger.error("signing file not found (todo: default signing config)");
            return;
        }

        replaceInFile("app/build.gradle", {
            'com.eliasku.template_android': ctx.android.application_id,
            'versionCode 1 // AUTO': `versionCode ${ctx.version_code} // AUTO`,
            'versionName "1.0" // AUTO': `versionName "${ctx.version_name}" // AUTO`,
            '// TEMPLATE_SOURCE_SETS': source_sets.join("\n\t\t"),
            '// TEMPLATE_DEPENDENCIES': android_dependency.join("\n\t"),
            'release {} /* ${SIGNING_CONFIGS} */': printSigningConfigs(signingConfig),
        });

        replaceInFile('fastlane/Appfile', {
            "__PACKAGE_NAME__": ctx.android.application_id,
            "__SERVICE_ACCOUNT_KEY_PATH__": serviceAccountKey,
        });

        writeFileSync('local.properties', `sdk.dir=${getAndroidSdkRoot()}`);

        copyFolderRecursiveSync(path.join(base_path, "export/android/res"), "app/src/main/res");

        mod_main_class(ctx.android.package_id);
        mod_android_manifest(ctx);
        createStringsXML(ctx);
        mod_cmake_lists(ctx);
        copy_google_services_config_android(googleServicesConfigDir);
    }

    // TODO: `build` instead of bundle
    if (ctx.args.indexOf("bundle") >= 0) {
        gradle('bundleRelease');
        process.chdir(cwd);
        const aabPath = path.join(dest_path, 'app/build/outputs/bundle/release/app-release.aab');
        if (isFile(aabPath)) {
            copyFile(path.join(dest_path, 'app/build/outputs/bundle/release/app-release.aab'),
                path.join(dest_dir, ctx.name + '_' + ctx.version_name + '.aab'));
        }
    }

    if (ctx.options?.openProject) {
        process.chdir(cwd);
        open_android_project(dest_path);
    }

    if (ctx.options.deploy != null) {
        execute("fastlane", [ctx.options.deploy], dest_path);
    }
}
