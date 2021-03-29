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
import {buildAssets} from "../assets";
import {collectSourceFiles, collectSourceRootsAll} from "../collectSources";
import {copySigningKeys, printSigningConfigs} from "./signing";
import {execSync} from "child_process";
import {androidBuildAppIcon} from "./androidAppIcon";
import * as fs from "fs";
import {Project} from "../project";
import {writeFileSync} from "fs";

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
        console.warn("missing google-services.json", config_file);
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
    let orientation = "sensorPortrait";
    if (ctx.orientation === "landscape") {
        orientation = "sensorLandscape";
    } else if (ctx.orientation === "portrait") {
        orientation = "sensorPortrait";
    } else {
        console.warn("unknown orientation", ctx.orientation);
    }
    const configChanges = "keyboardHidden|keyboard|orientation|screenSize|layoutDirection|locale|uiMode|screenLayout|smallestScreenSize|navigation";

    replaceInFile("app/src/main/AndroidManifest.xml", {
        'android:configChanges="PLACEHOLDER"': `android:configChanges="${configChanges}"`,
        "com.eliasku.template_android": ctx.android.package_id,
        'screenOrientation="sensorPortrait"': `screenOrientation="${orientation}"`,
        "<!-- TEMPLATE ROOT -->": ctx.build.android.add_manifest.join("\n"),
        "<!-- TEMPLATE APPLICATION -->": ctx.build.android.add_manifest_application.join("\n")
    });
}

function createStringsXML(ctx) {
    const doc = new XmlDocument(`<resources></resources>`);
    const xmlStrings = ctx.build.android.xmlStrings;
    xmlStrings.app_name = ctx.title;
    xmlStrings.package_name = ctx.android.application_id;
    for (let key of Object.keys(xmlStrings)) {
        const val = xmlStrings[key];
        doc.children.push(new XmlDocument(`<string name="${key}" translatable="false">${val}</string>`));
    }
    writeText("app/src/main/res/values/strings.xml", doc.toString());
}

function mod_cmake_lists(ctx) {
    const cmake_path = "CMakeLists.txt";
    const src_files = [];
    const ext_list = ["hpp", "hxx", "h", "cpp", "cxx", "c"];

    const source_dir_list = collectSourceRootsAll(ctx, "cpp", "android", ".");

    for (const source_dir of source_dir_list) {
        collectSourceFiles(source_dir, ext_list, src_files);
    }

    replaceInFile(cmake_path, {
        "#-SOURCES-#": src_files.join("\n\t\t"),
        "#-SEARCH_ROOTS-#": source_dir_list.join("\n\t\t")
    });
}


export function export_android(ctx: Project) {

    buildAssets(ctx);
    androidBuildAppIcon(ctx, "export/android/res");

    const platform_target = ctx.current_target; // "android"
    const platform_proj_name = ctx.name + "-" + ctx.current_target;
    const dest_dir = path.resolve(process.cwd(), "export");
    const dest_path = path.join(dest_dir, platform_proj_name);

    if (isDir(dest_path)) {
        console.info("Remove old project", dest_path);
        deleteFolderRecursive(dest_path);
        console.assert(!isDir(dest_path));
    } else {

    }

    // resolve absolute path to configs
    let googleServicesConfigDir = ctx.android.googleServicesConfigDir;
    let signingConfigPath = ctx.android.signingConfigPath;
    let serviceAccountKey = ctx.android.serviceAccountKey;
    if(googleServicesConfigDir) {
        googleServicesConfigDir = path.resolve(ctx.path.CURRENT_PROJECT_DIR, googleServicesConfigDir);
    }
    if(signingConfigPath) {
        signingConfigPath = path.resolve(ctx.path.CURRENT_PROJECT_DIR, signingConfigPath);
    }
    if(serviceAccountKey) {
        serviceAccountKey = path.resolve(ctx.path.CURRENT_PROJECT_DIR, serviceAccountKey);
    }

    copyFolderRecursiveSync(path.join(ctx.path.templates, `template-${platform_target}`), dest_path);
    const base_path = "../..";

    const cwd = process.cwd();
    process.chdir(dest_path);
    {
        const java_roots =
            collectSourceRootsAll(ctx, "java", "android", "app")
                .map((p) => `'${p}'`);

        const aidl_roots =
            collectSourceRootsAll(ctx, "aidl", "android", "app")
                .map((p) => `'${p}'`);

        const assets_roots =
            collectSourceRootsAll(ctx, "assets", "android", "app")
                .map((p) => `'${p}'`);

        const source_sets = [
            `main.java.srcDirs += [${java_roots.join(", ")}]`,
            `main.aidl.srcDirs += [${aidl_roots.join(", ")}]`,
            `main.assets.srcDirs += [${assets_roots.join(", ")}]`
        ];

        let signingConfig = {};
        let signingConfigBasePath = "";

        if (signingConfigPath) {
            const signingConfigJson = fs.readFileSync(signingConfigPath, "utf-8");
            signingConfig = JSON.parse(signingConfigJson);
            signingConfigBasePath = path.dirname(signingConfigPath);
            copySigningKeys(signingConfig, signingConfigBasePath);
        } else {
            console.error("signing file not found (todo: default signing config)");
            return;
        }

        replaceInFile("app/build.gradle", {
            'com.eliasku.template_android': ctx.android.application_id,
            'versionCode 1 // AUTO': `versionCode ${ctx.version_code} // AUTO`,
            'versionName "1.0" // AUTO': `versionName "${ctx.version_name}" // AUTO`,
            '// TEMPLATE_SOURCE_SETS': source_sets.join("\n\t\t"),
            '// TEMPLATE_DEPENDENCIES': ctx.build.android.dependencies.join("\n\t"),
            '// ${SIGNING_CONFIGS}': printSigningConfigs(signingConfig),
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

    if(ctx.options.deploy != null) {
        execute("fastlane", [ctx.options.deploy], dest_path);
    }
}
