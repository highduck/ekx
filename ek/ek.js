const path = require("path");
const fs = require("fs");
const glob = EK.require("glob");
const Mustache = EK.require("mustache");

const {spawnSync} = require('child_process');

function replace_all(str, search, replacement) {
    return str.split(search).join(replacement);
}

function execute(cmd, args) {
    console.debug(">> " + [cmd].concat(args).join(" "));
    const child = spawnSync(cmd, args);
    console.log("exit code", child.status);
    return child.status;
}

function open_android_project(android_project_path) {
    execute("open", ["-a", "/Applications/Android Studio.app", android_project_path]);
}

function read_text(src) {
    return fs.readFileSync(src, "utf8");
}

function write_text(filepath, text) {
    fs.writeFileSync(filepath, text, "utf8");
}

function copy_file(src, dest) {
    fs.copyFileSync(src, dest);
}

function is_dir(p) {
    return fs.existsSync(p) && fs.lstatSync(p).isDirectory();
}

function is_file(p) {
    return fs.existsSync(p) && fs.lstatSync(p).isFile();
}

function replace_in_file(filepath, dict) {
    let text = read_text(filepath);
    for (const [k, v] of Object.entries(dict)) {
        text = replace_all(text, k, v);
    }
    write_text(filepath, text);
}

function make_dirs(p) {
    // todo: improve (relax node version < 11)
    fs.mkdirSync(p, {recursive: true});
}

function search_files(pattern, search_path, out_files_list) {
    const files = glob.sync(pattern, {
        cwd: search_path
    });
    for (let file of files) {
        out_files_list.push(path.join(search_path, file));
    }
}

function copyFolderRecursiveSync(source, target) {
    if (!fs.existsSync(target)) {
        fs.mkdirSync(target);
    }

    //copy
    if (fs.lstatSync(source).isDirectory()) {
        fs.readdirSync(source).forEach(function (file) {
            var curSource = path.join(source, file);
            if (fs.lstatSync(curSource).isDirectory()) {
                copyFolderRecursiveSync(curSource, path.join(target, file));
            } else {
                fs.copyFileSync(curSource, path.join(target, file));
            }
        });
    }
}

var deleteFolderRecursive = function (p) {
    if (fs.existsSync(p)) {
        fs.readdirSync(p).forEach(function (file, index) {
            var curPath = p + "/" + file;
            if (fs.lstatSync(curPath).isDirectory()) { // recurse
                deleteFolderRecursive(curPath);
            } else { // delete file
                fs.unlinkSync(curPath);
            }
        });
        fs.rmdirSync(p);
    }
};

function copy_keystore(keystore_file) {
    const src = path.join("../..", keystore_file);
    if (is_file(src)) {
        copy_file(src, path.join("app", keystore_file));
    } else {
        console.warn("missing keystore", keystore_file);
    }
}

function copy_google_services_config_android() {
    const config_file = "google-services.json";
    const config_path = path.join("../..", config_file);
    if (is_file(config_path)) {
        copy_file(config_path, path.join("app", config_file));
    } else {
        console.warn("missing google-services.json", config_file);
    }
}

function mod_main_class(app_package_java) {
    const template_main_activity_java =
        "app/src/main/java/com/eliasku/template_android/MainActivity.java";
    const java_package_path = replace_all(app_package_java, ".", "/");

    let text = read_text(template_main_activity_java);
    text = replace_all(text,
        "package com.eliasku.template_android;",
        `package ${app_package_java};`
    );
    deleteFolderRecursive("app/src/main/java/com");
    const main_activity_path = path.join("app/src/main/java", java_package_path);
    make_dirs(main_activity_path);
    write_text(path.join(main_activity_path, "MainActivity.java"), text);
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

    replace_in_file("app/src/main/AndroidManifest.xml", {
        "com.eliasku.template_android": ctx.android.package_id,
        'screenOrientation="sensorPortrait"': `screenOrientation="${orientation}"`
    });
}

function mod_strings(ctx) {
    const xmldoc = EK.require("xmldoc");
    const res_strings_path = "app/src/main/res/values/strings.xml";
    const xml_text = read_text(res_strings_path);
    const doc = new xmldoc.XmlDocument(xml_text);
    // console.trace(doc.firstChild);
    // console.trace(doc.childrenNamed("string"));
    doc.eachChild((child) => {
        if (child.name === "string") {
            switch (child.attr.name) {
                case "app_name":
                    // seems xmldoc is not enough
                    child.val = ctx.title;
                    child.firstChild.text = ctx.title;
                    break;
                case "package_name":
                    child.firstChild.text = ctx.android.application_id;
                    break;
                case "gs_app_id":
                    child.firstChild.text = ctx.android.game_services_id;
                    break;
                case "admob_app_id":
                    child.firstChild.text = ctx.android.admob_app_id;
                    break;
            }
        }
    });
    // console.trace(doc.childrenNamed("string"));
    // console.trace(doc.toString());
    write_text(res_strings_path, doc.toString());
}

function collect_source_files(search_path, extensions, out_list = []) {
    for (const ext of extensions) {
        search_files("**/*." + ext, search_path, out_list);
    }
    return out_list;
}

function mod_cmake_lists(ctx) {
    const cmake_path = "CMakeLists.txt";
    const src_files = [];
    const ext_list = ["hpp", "h", "cpp", "c"];

    function core_path(p) {
        return path.relative(".", path.join(ctx.path.EKX_ROOT, p));
    }

    const source_dir_list = [
        "../../src",
        core_path("ecxx/src"),
        core_path("core/src"),
        core_path("ek/platforms/android"),
        core_path("ek/src"),
        core_path("scenex/src")
    ];

    for (const source_dir of source_dir_list) {
        collect_source_files(source_dir, ext_list, src_files);
    }

    replace_in_file(cmake_path, {
        "#-SOURCES-#": src_files.join("\n\t\t"),
        "#-SEARCH_ROOTS-#": source_dir_list.join("\n\t\t")
    });
}

function export_android(ctx) {
    const platform_target = ctx.current_target; // "android"
    const platform_proj_name = ctx.name + "-" + ctx.current_target;
    const dest_dir = "projects";
    const dest_path = path.join(dest_dir, platform_proj_name);

    if (is_dir(dest_path)) {
        console.info("Remove old project", dest_path);
        deleteFolderRecursive(dest_path);
        console.assert(!is_dir(dest_path));
    }

    copyFolderRecursiveSync(path.join(ctx.path.EKX_ROOT, "ek/templates/template-" + platform_target), dest_path);
    const base_path = "../..";
    const ek_path = path.join(ctx.path.EKX_ROOT, "ek");

    const cwd = process.cwd();
    process.chdir(dest_path);
    {
        const source_sets = [
            `main.java.srcDirs += '${path.join(ek_path, "platforms/android/java")}'`,
            `main.assets.srcDirs += 'src/main/assets'`
        ];

        replace_in_file("app/build.gradle", {
            'com.eliasku.template_android': ctx.android.application_id,
            'versionCode 1 // AUTO': `versionCode ${ctx.version_code} // AUTO`,
            'versionName "1.0" // AUTO': `versionName "${ctx.version_name}" // AUTO`,
            '// TEMPLATE_SOURCE_SETS': source_sets.join("\n\t\t"),
            'KEY_ALIAS': ctx.android.keystore.key_alias,
            'KEY_PASSWORD': ctx.android.keystore.key_password,
            'store.keystore': ctx.android.keystore.store_keystore,
            'STORE_PASSWORD': ctx.android.keystore.store_password
        });

        make_dirs("app/src/main/assets");
        copyFolderRecursiveSync(path.join(base_path, ctx.assets.output), "app/src/main/assets/assets");
        copyFolderRecursiveSync(path.join(base_path, "generated/android/res"), "app/src/main/res");

        mod_main_class(ctx.android.package_id);
        mod_android_manifest(ctx);
        mod_strings(ctx);
        mod_cmake_lists(ctx);
        copy_google_services_config_android();
        copy_keystore(ctx.android.keystore.store_keystore);
    }
    process.chdir(cwd);
    open_android_project(dest_path);
}

/*** iOS export ***/

function mod_plist(ctx, filepath) {
    const plist = EK.require("plist");
    const dict = plist.parse(read_text(filepath));
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

    dict["GADIsAdManagerApp"] = true;
    write_text(filepath, plist.build(dict));
}

// function xcode_disable_arc(xc, filepath) {
//     const rel_path = path.relative(filepath, ".");
//     print('Try Disable ARC for ' + rel_path);
//     for (file in project.get_files_by_path(rel_path)) {
//         for (build_file in project.get_build_files_for_file(file.get_id())) {
//             print("Disable ARC for: %s (%s)" % (file.name, build_file.get_id()));
//             build_file.add_compiler_flags('-fno-objc-arc');
//         }
//     }
// }
//
// function xcode_patch(ctx, proj_ios_name) {
//     const xcode = EK.require("xcode");
//
//     const application_id = ctx.ios.application_id;
//     const sdk_root = ctx.path.EKX_ROOT;
//
//     const xc_path = `${proj_ios_name}.xcodeproj/project.pbxproj`;
//     const xc = xcode.project(xc_path).parseSync();
//
//     const project_target = xc.pbxTargetByName("template-ios");
//     xc.addTargetAttribute("SystemCapabilities", {
//         "com.apple.GameCenter.iOS": {
//             enabled: 1
//         }
//     }, project_target);
//     xc.addTargetAttribute("SystemCapabilities", {
//         "com.apple.InAppPurchase": {
//             enabled: 1
//         }
//     }, project_target);
//     // project_target.name = proj_ios_name;
//     // project_target.productName = proj_ios_name;
//     // project_target.productReference_comment = proj_ios_name + ".app";
//     // sys_caps = PBXGenericObject()
//     // sys_caps["com.apple.GameCenter.iOS"] = PBXGenericObject()
//     // sys_caps["com.apple.GameCenter.iOS"]["enabled"] = 1
//     // sys_caps["com.apple.InAppPurchase"] = PBXGenericObject()
//     // sys_caps["com.apple.InAppPurchase"]["enabled"] = 1
//     // project.objects[project.rootObject].attributes.TargetAttributes[project_target.get_id()][
//     //     'SystemCapabilities'] = sys_caps
//
//     //# project.set_flags("DEBUG_INFORMATION_FORMAT", "dwarf-with-dsym")
//
//     xc.updateProductName(proj_ios_name);
//     //const project_targe2t = xc.pbxTargetByName(proj_ios_name);
//     //console.debug(project_targe2t);
//     // project_target.name = proj_ios_name
//     // project_target.productName = proj_ios_name
//     xc.updateBuildProperty("PRODUCT_BUNDLE_IDENTIFIER", application_id);
//     xc.updateBuildProperty("IPHONEOS_DEPLOYMENT_TARGET", "12.0");
//     // project.set_flags("PRODUCT_BUNDLE_IDENTIFIER", application_id)
//     // project.set_flags("IPHONEOS_DEPLOYMENT_TARGET", "12.0")
//
//     const src_files = [];
//     collect_source_files("../../src", src_files);
//     // collect_source_files(path.join(ctx.path.EKX_ROOT, "ecxx/src"), src_files);
//     // collect_source_files(path.join(ctx.path.EKX_ROOT, "core/src"), src_files);
//     // collect_source_files(path.join(ctx.path.EKX_ROOT, "ek/src"), src_files);
//     // collect_source_files(path.join(ctx.path.EKX_ROOT, "ek/platforms/apple"), src_files);
//     // collect_source_files(path.join(ctx.path.EKX_ROOT, "ek/platforms/ios"), src_files);
//     // collect_source_files(path.join(ctx.path.EKX_ROOT, "scenex/src"), src_files);
//
//     xc.addPbxGroup(src_files, "game_src");
//     // addPbxGroup = function(filePathsArray, name, path, sourceTree);
//     // for (const src of src_files) {
//     //     if (src.endsWith(".hpp") || src.endsWith(".h")) {
//     //         xc.addHeaderFile(src, {}, "game_src");
//     //     } else {
//     //         xc.addSourceFile(src, {}, "game_src");
//     //     }
//     // }
//     // project.add_folder('../../src', parent = project.add_group("src"), excludes = excludes)
//     // project.add_folder(sdk_root + '/ecxx/src', parent = project.add_group("ecxx"), excludes = excludes)
//     // project.add_folder(sdk_root + '/core/src', parent = project.add_group("ek-core"), excludes = excludes)
//     // platforms_group = project.add_group("ek-platforms")
//     // project.add_folder(sdk_root + '/ek/platforms/apple', parent = platforms_group, excludes = excludes)
//     // project.add_folder(sdk_root + '/ek/platforms/ios', parent = platforms_group, excludes = excludes)
//     // project.add_folder(sdk_root + '/ek/src', parent = project.add_group("ek"), excludes = excludes)
//     // project.add_folder(sdk_root + '/scenex/src', parent = project.add_group("scenex"), excludes = excludes)
//     //
//     // disable_arc(project, sdk_root + '/ek/platforms/ios/audiomini/SimpleAudioEngine_objc.mm')
//     // disable_arc(project, sdk_root + '/ek/platforms/ios/audiomini/CocosDenshion.mm')
//     // disable_arc(project, sdk_root + '/ek/platforms/ios/audiomini/CDOpenALSupport.mm')
//     // disable_arc(project, sdk_root + '/ek/platforms/ios/audiomini/CDAudioManager.mm')
//     // disable_arc(project, sdk_root + '/ek/platforms/ios/EAGLView.mm')
//     //
//
//     for (const search_path of [
//         "../../src",
//         sdk_root + "/ecxx/src",
//         sdk_root + "/core/src",
//         sdk_root + "/ek/platforms/apple",
//         sdk_root + "/ek/platforms/ios",
//         sdk_root + "/ek/src",
//         sdk_root + "/scenex/src"
//     ]) {
//         xc.addToHeaderSearchPaths(search_path);
//     }
//
//     //
//     // //# self.cpp_info.cxxflags.append("-fno-aligned-allocation")
//     const flags_frameworks = [
//         // "$(inherited)",
//         "-framework", "UIKit",
//         "-framework", "OpenGLES",
//         "-framework", "QuartzCore",
//         "-framework", "AudioToolbox",
//         "-framework", "Foundation",
//         "-framework", "OpenAL"
//     ];
//     //
//     // //# file_options = FileOptions(weak = True)
//     // file_options = FileOptions(weak = False, embed_framework = False);
//     // xc.addFramework("System/Library/Frameworks/GameKit.framework");
//     // xc.addFramework("System/Library/Frameworks/StoreKit.framework");
//     // project.add_file('System/Library/Frameworks/GameKit.framework', tree = 'SDKROOT', force = False, file_options = file_options);
//     // project.add_file('System/Library/Frameworks/StoreKit.framework', tree = 'SDKROOT', force = False, file_options = file_options);
//     //
//
//     for (const flag of flags_frameworks) {
//         xc.addToOtherLinkerFlags(flag);
//         //     xc.addToOtherLinkerFlags(flags_frameworks.join(" "));
//     }
//
//     // project.add_other_ldflags(" ".join(frameworks));
//     // project.add_library_search_paths("$(inherited)");
//     //
//     // project.add_other_cflags([
//     //     "$(inherited)",
//     //     "-DGLES_SILENCE_DEPRECATION"
//     // ]);
//
//     xc.addFile("assets");//, force = True);
//     xc.addFile("../../GoogleService-Info.plist");
//
//     fs.writeFileSync(xc_path, xc.writeSync());
// }

function export_ios(ctx) {
    const platform_target = ctx.current_target; // "ios"
    const platform_proj_name = ctx.name + "-" + platform_target;
    const dest_dir = "projects";
    const dest_path = path.join(dest_dir, platform_proj_name);

    if (is_dir(dest_path)) {
        console.info("Remove XCode project", dest_path);
        deleteFolderRecursive(dest_path);
        console.assert(!is_dir(dest_path));
    }

    copyFolderRecursiveSync(path.join(ctx.path.EKX_ROOT, "ek/templates/template-" + platform_target), dest_path);

    const base_path = "../..";
    const cwd = process.cwd();
    process.chdir(dest_path);
    {
        console.info("Rename project");
        fs.renameSync("template-ios.xcodeproj", platform_proj_name + ".xcodeproj");

        copyFolderRecursiveSync(path.join(base_path, ctx.assets.output), "assets");
        copyFolderRecursiveSync(path.join(base_path, "generated/ios/AppIcon.appiconset"),
            "src/Assets.xcassets/AppIcon.appiconset");

        const src_launch_logo_path = path.join(base_path, "generated/ios/AppIcon.appiconset");
        const dest_launch_logo_path = "src/Assets.xcassets/LaunchLogo.imageset";
        // launch logo
        copy_file(path.join(src_launch_logo_path, "iphone_40.png"),
            path.join(dest_launch_logo_path, "iphone_40.png"));
        copy_file(path.join(src_launch_logo_path, "iphone_80.png"),
            path.join(dest_launch_logo_path, "iphone_80.png"));
        copy_file(path.join(src_launch_logo_path, "iphone_120.png"),
            path.join(dest_launch_logo_path, "iphone_120.png"));

        mod_plist(ctx, "src/Info.plist");

        /// PRE MOD PROJECT
        //xcode_patch(ctx, platform_proj_name);
        execute("python3", ["xcode-project-ios.py", platform_proj_name, ctx.ios.application_id, ctx.path.EKX_ROOT]);

        console.info("Prepare PodFile");
        replace_in_file("Podfile", {
            "template-ios": platform_proj_name
        });

        console.info("Install Pods");
        execute("pod", ["install"]);

        // POST MOD PROJECT
        execute("python3", ["xcode-project-ios-post.py",
            platform_proj_name, ctx.ios.application_id]);
    }
    process.chdir(cwd);

    const workspace_path = path.join(dest_path, platform_proj_name + ".xcworkspace");
    // execute("open", [dest_path]);
    execute("open", [workspace_path]);
    // execute("xcodebuild", [
    //     "-workspace", workspace_path,
    //     "-scheme", platform_proj_name,
    //     "-configuration", "Release"
    // ]);
}

/*** HTML ***/
function export_web(ctx) {
    function tpl(from, to) {
        const tpl_text = fs.readFileSync(path.join(__dirname, from), "utf8");
        fs.writeFileSync(path.join(ctx.path.OUTPUT, to), Mustache.render(tpl_text, ctx), "utf8");
    }

    function file(from, to) {
        fs.copyFileSync(
            path.join(__dirname, from),
            path.join(ctx.path.OUTPUT, to)
        );
    }

    tpl("templates/web/index.html.mustache", "index.html");
    tpl("templates/web/manifest.json.mustache", "manifest.webmanifest");
    tpl("templates/web/sw.js.mustache", "sw.js");
    file("templates/web/howler.core.min.js", "howler.core.min.js");
    file("templates/web/pwacompat.min.js", "pwacompat.min.js");

    copyFolderRecursiveSync("generated/pwa/icons", path.join(ctx.path.OUTPUT, "icons"));
}

class File {
    constructor(ctx) {
        ctx.android = {};
        ctx.ios = {};
        ctx.html = {};

        console.log("=== EK PROJECT ===");
        console.log("Current Target:", ctx.current_target);

        const exporters = {
            web: export_web,
            android: export_android,
            ios: export_ios
        };
        const exporter = exporters[ctx.current_target];
        if (exporter) {
            ctx.build_steps.push(() => {
                exporter(ctx);
            });
        }
    }
}

module.exports = File;

//
//  void init_project_path(project_path_t& path) {
//     const auto* ekx_root = std::getenv("EKX_ROOT");
//     if (!ekx_root) {
//         EK_ERROR << "Please define EKX_ROOT environment variable. Abort.";
//         abort();
//     }
//
//     path.ekx = path_t{ekx_root};
//
//     if (!ek::is_dir(path.ekx)) {
//         EK_ERROR << "EKX_ROOT is not a directory: " << path.ekx;
//         abort();
//     }
//
//     path.emsdk = path_t{"/Users/ilyak/dev/emsdk"};
//     if (!ek::is_dir(path.emsdk)) {
//         EK_WARN << "Emscripten SDK dir is not found: " << path.ekx;
//         EK_WARN << "Web Target is not available";
//         path.emsdk_toolchain = path.emsdk /
//                                "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake";
//         if (!ek::is_file(path.emsdk_toolchain)) {
//             EK_WARN << "Emscripten SDK toolchain is not found: " << path.ekx;
//             EK_WARN << "Web Target is not available";
//         }
//     }
//
//     path.current_project = path.project = path_t{ek::current_working_directory()};
// }
