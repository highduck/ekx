import {
    copyFile,
    copyFolderRecursiveSync,
    deleteFolderRecursive,
    execute,
    isDir,
    readText,
    replaceInFile,
    writeText
} from "../utils";
import * as path from "path";
import * as fs from "fs";
import {readFileSync} from "fs";
import * as plist from 'plist';
import {buildAssetPackAsync} from "../assets";
import {iosBuildAppIconAsync} from "./iosAppIcon";
import {Project} from "../project";
import {collectCppFlags, collectObjects, collectStrings} from "../collectSources";
import {logger} from "../logger";

const iosPlatforms = ["apple", "ios"];

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

    const extra_data = collectObjects(ctx, "xcode_plist", iosPlatforms);
    for (const extra of extra_data) {
        for (const [k, v] of Object.entries(extra)) {
            dict[k] = v;
        }
    }
    writeText(filepath, plist.build(dict));
}

export async function export_ios(ctx: Project): Promise<void> {
    // setup automation
    let credentials: AppStoreCredentials = {};
    try {
        credentials = JSON.parse(readFileSync(ctx.ios.appStoreCredentials, "utf-8"));
    } catch (e) {
    }

    await Promise.all([
        buildAssetPackAsync(ctx),
        iosBuildAppIconAsync(ctx, "export/ios")
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

    copyFolderRecursiveSync(path.join(ctx.path.templates, "template-ios"), dest_path);

    const base_path = "../..";
    const cwd = process.cwd();
    process.chdir(dest_path);
    {
        // logger.info("Rename project");
        // fs.renameSync("app-ios.xcodeproj", platform_proj_name + ".xcodeproj");

        const embeddedAssetsDir = "assets";
        copyFolderRecursiveSync(path.join(base_path, ctx.getAssetsOutput()), embeddedAssetsDir);
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

        for(const fn of ctx.onProjectGenerated) {
            fn();
        }

        const xcode_projectPythonPostScript = collectStrings(ctx, "xcode_projectPythonPostScript", iosPlatforms, false);
        fs.writeFileSync("ek-ios-build.json", JSON.stringify({
            assets: collectStrings(ctx, "assets", iosPlatforms, true).concat([embeddedAssetsDir]),

            cpp: collectStrings(ctx, "cpp", iosPlatforms, true),
            cpp_include: collectStrings(ctx, "cpp_include", iosPlatforms, true),
            cpp_lib: collectStrings(ctx, "cpp_lib", iosPlatforms, false),
            cpp_define: collectStrings(ctx, "cpp_define", iosPlatforms, false),
            cpp_flags: collectCppFlags(ctx, iosPlatforms),

            xcode_framework: collectStrings(ctx, "xcode_framework", iosPlatforms, false),
            xcode_capability: collectStrings(ctx, "xcode_capability", iosPlatforms, false),
            xcode_plist: collectObjects(ctx, "xcode_plist", iosPlatforms),
            xcode_pod: collectStrings(ctx, "xcode_pod", iosPlatforms, false),
            xcode_file: collectStrings(ctx, "xcode_file", iosPlatforms, false)
        }));

        /// PRE MOD PROJECT
        //xcode_patch(ctx, platform_proj_name);
        execute("python3", ["xcode-project-ios.py", platform_proj_name, ctx.ios.application_id]);

        logger.info("Prepare PodFile");
        const pods = collectStrings(ctx, "xcode_pod", iosPlatforms, false)
            .map((v) => `pod '${v}'`).join("\n  ");
        replaceInFile("Podfile", {
            // "app-ios": platform_proj_name,
            "# TEMPLATE DEPENDENCIES": pods
        });

        replaceInFile("fastlane/Appfile", {
            "[[APP_IDENTIFIER]]": ctx.ios.application_id,
            "[[APPLE_ID]]": credentials.apple_id ?? "",
            "[[TEAM_ID]]": credentials.team_id ?? "",
            "[[ITC_TEAM_ID]]": credentials.itc_team_id ?? "",
        });

        logger.info("Install Pods");
        execute("pod", ["install", "--repo-update"]);

        // POST MOD PROJECT
        replaceInFile("xcode-project-ios-post.py", {
            "# XCODE_POST_PROJECT": xcode_projectPythonPostScript.join("\n")
        });

        execute("python3", ["xcode-project-ios-post.py",
            platform_proj_name, ctx.ios.application_id]);
    }
    process.chdir(cwd);

    if (ctx.options.openProject) {
        const workspace_path = path.join(dest_path, "app-ios.xcworkspace");
        execute("open", [workspace_path]);
        // execute("xcodebuild", [
        //     "-workspace", workspace_path,
        //     "-scheme", platform_proj_name,
        //     "-configuration", "Release"
        // ]);
    }

    if (ctx.options.deploy != null) {
        execute("fastlane", [ctx.options.deploy], dest_path, {
            FASTLANE_APPLE_APPLICATION_SPECIFIC_PASSWORD: credentials.application_specific_password
        });
    }
}