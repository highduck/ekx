import {
    copyFolderRecursiveSync,
    deleteFolderRecursive,
    execute2,
    isDir,
    replaceInFile
} from "../utils.js";
import plist from "plist";
import * as path from "path";
import * as fs from "fs";
import {buildAssetPackAsync} from "../assets.js";
import {Project} from "../project.js";
import {collectCppFlags, collectObjects, collectStrings} from "../collectSources.js";
import {logger} from "../logger.js";
import {buildAppIconAsync} from "../appicon/appicon.js";
import {readTextFileSync, callInDir, writeTextFileSync} from "../../utils/utils.js";

const iosPlatforms = ["apple", "ios"];

interface AppStoreCredentials {
    team_id?: string;
    apple_id?: string;
    itc_team_id?: string;
    application_specific_password?: string;
}

function mod_plist(ctx: Project, filepath: string) {
    const dict:any = plist.parse(readTextFileSync(filepath));
    dict["CFBundleDisplayName"] = ctx.title;
    dict["CFBundleShortVersionString"] = ctx.version.name();
    dict["CFBundleVersion"] = "" + ctx.version.buildNumber();
    dict["UIRequiresFullScreen"] = true;
    dict["UIStatusBarHidden"] = true;
    dict["UIStatusBarStyle"] = "UIStatusBarStyleDefault";
    dict["UIViewControllerBasedStatusBarAppearance"] = false;

    if (ctx.orientation === "portrait") {
        dict["UISupportedInterfaceOrientations"] = [
            "UIInterfaceOrientationPortrait"
        ];
        dict["UISupportedInterfaceOrientations~ipad"] = [
            "UIInterfaceOrientationPortrait",
            "UIInterfaceOrientationPortraitUpsideDown"
        ];
    } else {
        dict["UISupportedInterfaceOrientations"] = [
            "UIInterfaceOrientationLandscapeLeft",
            "UIInterfaceOrientationLandscapeRight"
        ];
        dict["UISupportedInterfaceOrientations~ipad"] = [
            "UIInterfaceOrientationLandscapeLeft",
            "UIInterfaceOrientationLandscapeRight"
        ];
    }

    const extra_data = collectObjects(ctx, "xcode_plist", iosPlatforms);
    for (const extra of extra_data) {
        for (const [k, v] of Object.entries(extra)) {
            dict[k] = v;
        }
    }
    writeTextFileSync(filepath, plist.build(dict));
}

export async function export_ios(ctx: Project): Promise<void> {

    ctx.generateNativeBuildInfo();

    // setup automation
    let credentials: AppStoreCredentials = {};
    try {
        credentials = JSON.parse(readTextFileSync(ctx.ios.appStoreCredentials!));
    } catch {
        // ignore
    }

    const iconsContents = JSON.parse(readTextFileSync(path.join(ctx.sdk.templates, "template-ios/src/Assets.xcassets/AppIcon.appiconset/Contents.json")));
    await Promise.all([
        buildAssetPackAsync(ctx),
        buildAppIconAsync({
            projectType: "ios",
            iosAppIconContents: iconsContents,
            output: "export/ios",
            iconPath: ctx.appIcon
        })
    ]);

    const platform_target = ctx.current_target; // "ios"
    const platform_proj_name = ctx.name + "-" + platform_target;
    const dest_dir = path.resolve(process.cwd(), "export");
    const dest_path = path.join(dest_dir, platform_proj_name);

    if (isDir(dest_path)) {
        logger.info("Remove XCode project", dest_path);
        deleteFolderRecursive(dest_path);
        logger.assert(!isDir(dest_path));
    }

    copyFolderRecursiveSync(path.join(ctx.sdk.templates, "template-ios"), dest_path);

    const base_path = "../..";
    await callInDir(dest_path, async ()=>{
        const embeddedAssetsDir = "assets";
        copyFolderRecursiveSync(path.join(base_path, ctx.getAssetsOutput()), embeddedAssetsDir);
        copyFolderRecursiveSync(path.join(base_path, "export/ios/AppIcon.appiconset"),
            "src/Assets.xcassets/AppIcon.appiconset");

        const src_launch_logo_path = path.join(base_path, "export/ios/AppIcon.appiconset");
        const dest_launch_logo_path = "src/Assets.xcassets/LaunchLogo.imageset";
        // launch logo
        fs.copyFileSync(path.join(src_launch_logo_path, "iphone_40.png"),
            path.join(dest_launch_logo_path, "iphone_40.png"));
        fs.copyFileSync(path.join(src_launch_logo_path, "iphone_80.png"),
            path.join(dest_launch_logo_path, "iphone_80.png"));
        fs.copyFileSync(path.join(src_launch_logo_path, "iphone_120.png"),
            path.join(dest_launch_logo_path, "iphone_120.png"));

        mod_plist(ctx, "src/Info.plist");

        for (const fn of ctx.onProjectGenerated) {
            fn();
        }

        const xcode_projectPythonPostScript = collectStrings(ctx, "xcode_projectPythonPostScript", iosPlatforms, false);
        const declaration: any = {modules: []};
        for (const module of ctx.modules) {
            declaration.modules.push({
                name: module.name ?? (module.path ? path.basename(module.path) : "global"),

                assets: collectStrings(module, "assets", iosPlatforms, true),

                cpp: collectStrings(module, "cpp", iosPlatforms, true),
                cpp_include: collectStrings(module, "cpp_include", iosPlatforms, true),
                cpp_lib: collectStrings(module, "cpp_lib", iosPlatforms, false),
                cpp_define: collectStrings(module, "cpp_define", iosPlatforms, false),
                cpp_flags: collectCppFlags(module, iosPlatforms),

                xcode_framework: collectStrings(module, "xcode_framework", iosPlatforms, false),
                xcode_capability: collectStrings(module, "xcode_capability", iosPlatforms, false),
                xcode_plist: collectObjects(module, "xcode_plist", iosPlatforms),
                xcode_pod: collectStrings(module, "xcode_pod", iosPlatforms, false),
                xcode_file: collectStrings(module, "xcode_file", iosPlatforms, false)
            });
        }
        declaration.modules.push({name: "embedded", assets: [embeddedAssetsDir]});

        writeTextFileSync("ek-ios-build.json", JSON.stringify(declaration));

        /// PRE MOD PROJECT
        //xcode_patch(ctx, platform_proj_name);
        await execute2("python3", ["xcode-project-ios.py", platform_proj_name, ctx.ios.application_id!]);

        logger.info("Prepare PodFile");
        const pods = collectStrings(ctx, "xcode_pod", iosPlatforms, false)
            .map((v) => `pod '${v}'`).join("\n  ");
        replaceInFile("Podfile", {
            // "app-ios": platform_proj_name,
            "# TEMPLATE DEPENDENCIES": pods
        });

        replaceInFile("fastlane/Appfile", {
            "[[APP_IDENTIFIER]]": ctx.ios.application_id!,
            "[[APPLE_ID]]": credentials.apple_id ?? "",
            "[[TEAM_ID]]": credentials.team_id ?? "",
            "[[ITC_TEAM_ID]]": credentials.itc_team_id ?? "",
        });

        logger.info("Install Pods");
        if (0 !== await execute2("pod", ["install", "--repo-update"])) {
            // maybe no internet connection, so we can't update pods repo
            await execute2("pod", ["install"]);
        }

        // POST MOD PROJECT
        replaceInFile("xcode-project-ios-post.py", {
            "# XCODE_POST_PROJECT": xcode_projectPythonPostScript.join("\n")
        });

        await execute2("python3", ["xcode-project-ios-post.py",
            platform_proj_name, ctx.ios.application_id!]);
    });

    if (ctx.options.openProject) {
        const workspace_path = path.join(dest_path, "app-ios.xcworkspace");
        await execute2("open", [workspace_path]);
        // execute("xcodebuild", [
        //     "-workspace", workspace_path,
        //     "-scheme", platform_proj_name,
        //     "-configuration", "Release"
        // ]);
    }

    if (ctx.options.deploy != null) {
        await execute2("fastlane", [ctx.options.deploy], dest_path, {
            FASTLANE_APPLE_APPLICATION_SPECIFIC_PASSWORD: credentials.application_specific_password!
        });
    }
}