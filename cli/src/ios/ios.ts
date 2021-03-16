import {
    copyFile,
    copyFolderRecursiveSync,
    deleteFolderRecursive, execute,
    isDir,
    readText, replaceInFile,
    writeText
} from "../utils";
import * as path from "path";
import * as fs from "fs";
import * as plist from 'plist';
import {buildAssets, buildMarketingAssets} from "../assets";
import {iosBuildAppIcon} from "./iosAppIcon";
import {Project} from "../project";
import {readFileSync} from "fs";
import {spawnSync} from "child_process";

interface AppStoreCredentials {
    team_id?: string;
    apple_id?: string;
    itc_team_id?: string;
    application_specific_password?: string;
}

function mod_plist(ctx, filepath) {
    const dict = plist.parse(readText(filepath));
    dict["CFBundleDisplayName"] = ctx.title;
    dict["CFBundleShortVersionString"] = ctx.version_name;
    dict["CFBundleVersion"] = ctx.version_code;
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

    const extra_data = collect_xcode_props(ctx, "plist", "ios");
    for (const extra of extra_data) {
        for (const [k, v] of Object.entries(extra)) {
            dict[k] = v;
        }
    }
    writeText(filepath, plist.build(dict));
}

function get_pods(data) {
    let pods = [];
    if (data.xcode && data.xcode.pods) {
        pods = data.xcode.pods;
    }
    return pods;
}

function collect_pods(ctx) {
    let pods = [];
    for (const data of ctx.modules) {
        pods = pods.concat(get_pods(data));
        if (data.ios) {
            pods = pods.concat(get_pods(data.ios));
        }
        if (data.macos) {
            pods = pods.concat(get_pods(data.macos));
        }
    }
    return pods;
}

function get_module_data_prop(data, kind, prop) {
    return (data && data[kind] && data[kind][prop]) ? data[kind][prop] : [];
}

function collect_xcode_props(ctx, prop, target) {
    let list = [];
    for (const data of ctx.modules) {
        list = list.concat(get_module_data_prop(data, "xcode", prop));
        list = list.concat(get_module_data_prop(data[target], "xcode", prop));
    }
    return list;
}

export function export_ios(ctx: Project) {
    // setup automation
    let credentials: AppStoreCredentials = {};
    try {
        credentials = JSON.parse(readFileSync(ctx.ios.appStoreCredentials, "utf-8"));
    } catch (e) {
    }

    buildAssets(ctx);
    iosBuildAppIcon(ctx, "export/ios");

    const platform_target = ctx.current_target; // "ios"
    const platform_proj_name = ctx.name + "-" + platform_target;
    const dest_dir = path.resolve(process.cwd(), "export");
    const dest_path = path.join(dest_dir, platform_proj_name);

    if (isDir(dest_path)) {
        console.info("Remove XCode project", dest_path);
        deleteFolderRecursive(dest_path);
        console.assert(!isDir(dest_path));
    }

    copyFolderRecursiveSync(path.join(ctx.path.templates, "template-" + platform_target), dest_path);

    const base_path = "../..";
    const cwd = process.cwd();
    process.chdir(dest_path);
    {
        console.info("Rename project");
        fs.renameSync("template-ios.xcodeproj", platform_proj_name + ".xcodeproj");

        copyFolderRecursiveSync(path.join(base_path, ctx.getAssetsOutput()), "assets");
        copyFolderRecursiveSync(path.join(base_path, "export/ios/AppIcon.appiconset"),
            "src/Assets.xcassets/AppIcon.appiconset");

        const src_launch_logo_path = path.join(base_path, "export/ios/AppIcon.appiconset");
        const dest_launch_logo_path = "src/Assets.xcassets/LaunchLogo.imageset";
        // launch logo
        copyFile(path.join(src_launch_logo_path, "iphone_40.png"),
            path.join(dest_launch_logo_path, "iphone_40.png"));
        copyFile(path.join(src_launch_logo_path, "iphone_80.png"),
            path.join(dest_launch_logo_path, "iphone_80.png"));
        copyFile(path.join(src_launch_logo_path, "iphone_120.png"),
            path.join(dest_launch_logo_path, "iphone_120.png"));

        mod_plist(ctx, "src/Info.plist");
        fs.writeFileSync("ek-ios-build.json", JSON.stringify({
            modules: ctx.modules
        }));

        if (ctx.ios.googleServicesConfigDir) {
            copyFile(path.join(ctx.ios.googleServicesConfigDir, "GoogleService-Info.plist"), "GoogleService-Info.plist");
        }

        /// PRE MOD PROJECT
        //xcode_patch(ctx, platform_proj_name);
        execute("python3", ["xcode-project-ios.py", platform_proj_name, ctx.ios.application_id, ctx.path.EKX_ROOT]);

        console.info("Prepare PodFile");
        const pods = collect_xcode_props(ctx, "pods", "ios").map((v) => `pod '${v}'`).join("\n  ");
        replaceInFile("Podfile", {
            "template-ios": platform_proj_name,
            "# TEMPLATE DEPENDENCIES": pods
        });

        replaceInFile("fastlane/Appfile", {
            "[[APP_IDENTIFIER]]": ctx.ios.application_id,
            "[[APPLE_ID]]": credentials.apple_id ?? "",
            "[[TEAM_ID]]": credentials.team_id ?? "",
            "[[ITC_TEAM_ID]]": credentials.itc_team_id ?? "",
        });

        console.info("Install Pods");
        execute("pod", ["install"]);

        // POST MOD PROJECT
        execute("python3", ["xcode-project-ios-post.py",
            platform_proj_name, ctx.ios.application_id]);
    }
    process.chdir(cwd);

    if (ctx.options.openProject) {
        const workspace_path = path.join(dest_path, platform_proj_name + ".xcworkspace");
        // execute("open", [dest_path]);
        execute("open", [workspace_path]);
        // execute("xcodebuild", [
        //     "-workspace", workspace_path,
        //     "-scheme", platform_proj_name,
        //     "-configuration", "Release"
        // ]);
    }

    if (ctx.options.deployBeta) {
        execute("fastlane", ["beta"], dest_path, {
            FASTLANE_APPLE_APPLICATION_SPECIFIC_PASSWORD: credentials.application_specific_password
        });
    }
}