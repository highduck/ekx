import {
    copyFile,
    copyFolderRecursiveSync,
    deleteFolderRecursive,
    execute,
    isDir,
    readText,
    replaceInFile,
    writeText
} from "../utils.ts";
import {path, plist} from "../../deps.ts";
import {buildAssetPackAsync} from "../assets.ts";
import {Project} from "../project.ts";
import {collectCppFlags, collectObjects, collectStrings} from "../collectSources.ts";
import {logger} from "../logger.ts";
import {buildAppIconAsync} from "../appicon/appicon.ts";

const iosPlatforms = ["apple", "ios"];

interface AppStoreCredentials {
    team_id?: string;
    apple_id?: string;
    itc_team_id?: string;
    application_specific_password?: string;
}

function mod_plist(ctx: Project, filepath: string) {
    const dict = plist.parse(readText(filepath));
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
    writeText(filepath, plist.build(dict));
}

export async function export_ios(ctx: Project): Promise<void> {

    ctx.generateNativeBuildInfo();

    // setup automation
    let credentials: AppStoreCredentials = {};
    try {
        credentials = JSON.parse(Deno.readTextFileSync(ctx.ios.appStoreCredentials!));
    } catch {
        // ignore
    }

    const iconsContents = JSON.parse(readText(path.join(ctx.sdk.templates, "template-ios/src/Assets.xcassets/AppIcon.appiconset/Contents.json")));
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
    const dest_dir = path.resolve(Deno.cwd(), "export");
    const dest_path = path.join(dest_dir, platform_proj_name);

    if (isDir(dest_path)) {
        logger.info("Remove XCode project", dest_path);
        deleteFolderRecursive(dest_path);
        logger.assert(!isDir(dest_path));
    }

    copyFolderRecursiveSync(path.join(ctx.sdk.templates, "template-ios"), dest_path);

    const base_path = "../..";
    const cwd = Deno.cwd();
    Deno.chdir(dest_path);
    {
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

        Deno.writeTextFileSync("ek-ios-build.json", JSON.stringify(declaration));

        /// PRE MOD PROJECT
        //xcode_patch(ctx, platform_proj_name);
        await execute("python3", ["xcode-project-ios.py", platform_proj_name, ctx.ios.application_id!]);

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
        if (0 !== await execute("pod", ["install", "--repo-update"])) {
            // maybe no internet connection, so we can't update pods repo
            await execute("pod", ["install"]);
        }

        // POST MOD PROJECT
        replaceInFile("xcode-project-ios-post.py", {
            "# XCODE_POST_PROJECT": xcode_projectPythonPostScript.join("\n")
        });

        await execute("python3", ["xcode-project-ios-post.py",
            platform_proj_name, ctx.ios.application_id!]);
    }
    Deno.chdir(cwd);

    if (ctx.options.openProject) {
        const workspace_path = path.join(dest_path, "app-ios.xcworkspace");
        await execute("open", [workspace_path]);
        // execute("xcodebuild", [
        //     "-workspace", workspace_path,
        //     "-scheme", platform_proj_name,
        //     "-configuration", "Release"
        // ]);
    }

    if (ctx.options.deploy != null) {
        await execute("fastlane", [ctx.options.deploy], dest_path, {
            FASTLANE_APPLE_APPLICATION_SPECIFIC_PASSWORD: credentials.application_specific_password!
        });
    }
}