const path = require("path");
const fs = require("fs");
const glob = EK.require("glob");
const Mustache = EK.require("mustache");

function add_roots(to_roots, from_data) {
    for (src_type of ["cpp", "java", "js"]) {
        const from_data_src = from_data[src_type];
        if (from_data_src) {
            to_roots[src_type] = (to_roots[src_type] || []).concat(from_data_src);
        }
    }
}

function get_source_roots(ctx) {
    const roots = {
        "cpp": [],
        "java": [],
        "js": []
    };
    const target = ctx.current_target;
    for (module_info of ctx.modules) {
        console.log(module_info.name);
        add_roots(roots, module_info);
        if (module_info[target]) {
            add_roots(roots, module_info[target]);
        }
    }
    return roots;
}

function replace_all(str, search, replacement) {
    return str.split(search).join(replacement);
}

function ekc_export_market(ctx, target_type, output) {
    make_dirs(output);
    EK.execute(path.join(ctx.path.EKX_ROOT, "editor/bin/ekc"), ["export", "market", ctx.market_asset, target_type, output]);
}

function ekc_export_assets(ctx) {
    let assets_input = "assets";
    let assets_output = ctx.assets.output;
    make_dirs(assets_output);
    EK.execute(path.join(ctx.path.EKX_ROOT, "editor/bin/ekc"), ["export", "assets", assets_input, assets_output]);
    EK.optimize_png_glob(path.join(assets_output, "*.png"));
}

function ekc_export_assets_lazy(ctx) {
    let assets_output = ctx.assets.output;
    if (!is_dir(assets_output)) {
        ekc_export_assets(ctx);
    }
}

function open_android_project(android_project_path) {
    EK.execute("open", ["-a", "/Applications/Android Studio.app", android_project_path]);
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
    if (!is_dir(p)) {
        fs.mkdirSync(p, {recursive: true});
    }
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
    make_dirs(target);

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
        'screenOrientation="sensorPortrait"': `screenOrientation="${orientation}"`,
        "<!-- TEMPLATE ROOT -->": ctx.build.android.add_manifest.join("\n"),
        "<!-- TEMPLATE APPLICATION -->": ctx.build.android.add_manifest_application.join("\n")
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

// src_kind - "cpp", "java", "js", etc..
function collect_src_roots(data, src_kind) {
    let result = [];
    if (data && data[src_kind]) {
        result = result.concat(data[src_kind]);
    }
    return result;
}

// rel_to - optional, for example "." relative to current working directory
function collect_src_roots_all(ctx, src_kind, extra_target, rel_to) {
    let result = [];
    for (data of ctx.modules) {
        result = result.concat(collect_src_roots(data, src_kind));
        if (extra_target) {
            result = result.concat(collect_src_roots(data[extra_target], src_kind));
        }
    }
    if (rel_to) {
        result = result.map((p) => path.relative(rel_to, p));
    }
    return result;
}

function mod_cmake_lists(ctx) {
    const cmake_path = "CMakeLists.txt";
    const src_files = [];
    const ext_list = ["hpp", "h", "cpp", "c"];

    const source_dir_list = collect_src_roots_all(ctx, "cpp", "android", ".");

    for (const source_dir of source_dir_list) {
        collect_source_files(source_dir, ext_list, src_files);
    }

    replace_in_file(cmake_path, {
        "#-SOURCES-#": src_files.join("\n\t\t"),
        "#-SEARCH_ROOTS-#": source_dir_list.join("\n\t\t")
    });
}

function export_android(ctx) {

    ekc_export_assets_lazy(ctx);
    if (!is_dir("export/android/res")) {
        ekc_export_market(ctx, "android", "export/android/res");
        EK.optimize_png_glob("export/android/res/**/*.png");
    }

    const platform_target = ctx.current_target; // "android"
    const platform_proj_name = ctx.name + "-" + ctx.current_target;
    const dest_dir = "export";
    const dest_path = path.join(dest_dir, platform_proj_name);

    if (is_dir(dest_path)) {
        console.info("Remove old project", dest_path);
        deleteFolderRecursive(dest_path);
        console.assert(!is_dir(dest_path));
    } else {

    }

    copyFolderRecursiveSync(path.join(ctx.path.EKX_ROOT, "ek/templates/template-" + platform_target), dest_path);
    const base_path = "../..";

    const cwd = process.cwd();
    process.chdir(dest_path);
    {
        const java_roots =
            collect_src_roots_all(ctx, "java", "android", "app")
                .map((p) => `'${p}'`);

        const assets_roots =
            collect_src_roots_all(ctx, "assets", "android", "app")
                .map((p) => `'${p}'`);

        const source_sets = [
            `main.java.srcDirs += [${java_roots.join(", ")}]`,
            `main.assets.srcDirs += [${assets_roots.join(", ")}]`
        ];

        replace_in_file("app/build.gradle", {
            'com.eliasku.template_android': ctx.android.application_id,
            'versionCode 1 // AUTO': `versionCode ${ctx.version_code} // AUTO`,
            'versionName "1.0" // AUTO': `versionName "${ctx.version_name}" // AUTO`,
            '// TEMPLATE_SOURCE_SETS': source_sets.join("\n\t\t"),
            '// TEMPLATE_DEPENDENCIES': ctx.build.android.dependencies.join("\n\t"),
            'KEY_ALIAS': ctx.android.keystore.key_alias,
            'KEY_PASSWORD': ctx.android.keystore.key_password,
            'store.keystore': ctx.android.keystore.store_keystore,
            'STORE_PASSWORD': ctx.android.keystore.store_password
        });

        copyFolderRecursiveSync(path.join(base_path, "export/android/res"), "app/src/main/res");

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

    const extra_data = collect_xcode_props(ctx, "plist", "ios");
    for (const extra of extra_data) {
        for (const [k, v] of Object.entries(extra)) {
            dict[k] = v;
        }
    }
    write_text(filepath, plist.build(dict));
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
    for (data of ctx.modules) {
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
    for (data of ctx.modules) {
        list = list.concat(get_module_data_prop(data, "xcode", prop));
        list = list.concat(get_module_data_prop(data[target], "xcode", prop));
    }
    return list;
}

function export_ios(ctx) {
    ekc_export_assets_lazy(ctx);
    if (!is_dir("export/ios")) {
        ekc_export_market(ctx, "ios", "export/ios");
        EK.optimize_png_glob("export/ios/**/*.png");
    }

    const platform_target = ctx.current_target; // "ios"
    const platform_proj_name = ctx.name + "-" + platform_target;
    const dest_dir = "export";
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
        copyFolderRecursiveSync(path.join(base_path, "export/ios/AppIcon.appiconset"),
            "src/Assets.xcassets/AppIcon.appiconset");

        const src_launch_logo_path = path.join(base_path, "export/ios/AppIcon.appiconset");
        const dest_launch_logo_path = "src/Assets.xcassets/LaunchLogo.imageset";
        // launch logo
        copy_file(path.join(src_launch_logo_path, "iphone_40.png"),
            path.join(dest_launch_logo_path, "iphone_40.png"));
        copy_file(path.join(src_launch_logo_path, "iphone_80.png"),
            path.join(dest_launch_logo_path, "iphone_80.png"));
        copy_file(path.join(src_launch_logo_path, "iphone_120.png"),
            path.join(dest_launch_logo_path, "iphone_120.png"));

        mod_plist(ctx, "src/Info.plist");
        fs.writeFileSync("ek-ios-build.json", JSON.stringify({
            modules: ctx.modules
        }));

        /// PRE MOD PROJECT
        //xcode_patch(ctx, platform_proj_name);
        EK.execute("python3", ["xcode-project-ios.py", platform_proj_name, ctx.ios.application_id, ctx.path.EKX_ROOT]);

        console.info("Prepare PodFile");
        const pods = collect_xcode_props(ctx, "pods", "ios").map((v) => `pod '${v}'`).join("\n  ");
        replace_in_file("Podfile", {
            "template-ios": platform_proj_name,
            "# TEMPLATE DEPENDENCIES": pods
        });

        console.info("Install Pods");
        EK.execute("pod", ["install"]);

        // POST MOD PROJECT
        EK.execute("python3", ["xcode-project-ios-post.py",
            platform_proj_name, ctx.ios.application_id]);
    }
    process.chdir(cwd);

    const workspace_path = path.join(dest_path, platform_proj_name + ".xcworkspace");
    // execute("open", [dest_path]);
    EK.execute("open", [workspace_path]);
    // execute("xcodebuild", [
    //     "-workspace", workspace_path,
    //     "-scheme", platform_proj_name,
    //     "-configuration", "Release"
    // ]);
}

/*** HTML ***/
function export_web(ctx) {
    const output_dir = ctx.path.CURRENT_PROJECT_DIR + "/export/web";

    function tpl(from, to) {
        const tpl_text = fs.readFileSync(path.join(__dirname, from), "utf8");
        fs.writeFileSync(path.join(output_dir, to), Mustache.render(tpl_text, ctx), "utf8");
    }

    function file(from, to) {
        fs.copyFileSync(
            path.join(__dirname, from),
            path.join(output_dir, to)
        );
    }

    ekc_export_assets_lazy(ctx);
    if (!is_dir(path.join(output_dir, "icons"))) {
        ekc_export_market(ctx, "web", path.join(output_dir, "icons"));
        EK.optimize_png_glob(path.join(output_dir, "icons/*.png"));
    }

    tpl("templates/web/index.html.mustache", "index.html");
    tpl("templates/web/manifest.json.mustache", "manifest.webmanifest");
    tpl("templates/web/sw.js.mustache", "sw.js");
    file("templates/web/howler.core.min.js", "howler.core.min.js");
    file("templates/web/pwacompat.min.js", "pwacompat.min.js");

    copyFolderRecursiveSync("export/contents/assets", "export/web/assets");
}

class File {
    constructor(ctx) {
        ctx.market_asset = "assets/res";
        ctx.modules = [
            {
                name: "app",
                cpp: [path.join(__dirname, "src")],
                android: {
                    cpp: [path.join(__dirname, "platforms/android")],
                    java: [path.join(__dirname, "platforms/android/java")]
                },
                macos: {
                    cpp: [
                        path.join(__dirname, "platforms/apple"),
                        path.join(__dirname, "platforms/mac")
                    ]
                },
                ios: {
                    cpp: [
                        path.join(__dirname, "platforms/apple"),
                        path.join(__dirname, "platforms/ios")
                    ],
                    cpp_flags: {
                        files: [
                            path.join(__dirname, "platforms/ios/cocos-audio/SimpleAudioEngine_objc.mm"),
                            path.join(__dirname, "platforms/ios/cocos-audio/CocosDenshion.mm"),
                            path.join(__dirname, "platforms/ios/cocos-audio/CDOpenALSupport.mm"),
                            path.join(__dirname, "platforms/ios/cocos-audio/CDAudioManager.mm"),
                        ],
                        flags: "-fno-objc-arc"
                    },
                    xcode: {
                        capabilities: ["com.apple.GameCenter"],
                        frameworks: [
                            "UIKit", "OpenGLES", "QuartzCore", "Foundation",
                            "OpenAL", "AudioToolbox", "AVFoundation",
                            "GameKit"
                        ],
                        pods: [
                            "Firebase/Analytics",
                            "Fabric",
                            "Crashlytics"
                        ]
                    }
                },
                web: {
                    cpp: [path.join(__dirname, "platforms/web")]
                },
                windows: {
                    cpp: [path.join(__dirname, "platforms/windows")]
                },
                linux: {
                    cpp: [path.join(__dirname, "platforms/linux")]
                }
            }
        ];
        ctx.build = {
            android: {
                dependencies: [],
                add_manifest: [],
                add_manifest_application: [],
                source_dirs: [],
            }
        };
        ctx.ios = {};
        ctx.html = {};

        console.log("=== EK PROJECT ===");
        console.log("Current Target:", ctx.current_target);
        console.log("Module Path:", __dirname);

        const exporters = {
            web: export_web,
            android: export_android,
            ios: export_ios,
            market: () => {
                ekc_export_market(ctx, "gen", "export/market");
            },
            assets: () => {
                ekc_export_assets(ctx);
            }
        };
        const exporter = exporters[ctx.current_target];
        if (exporter) {
            ctx.build_steps.push(() => {
                exporter(ctx);
            });

            ctx.build_steps.push(() => {
                const roots = get_source_roots(ctx);
                console.log(roots);
            });
        }
    }
}

module.exports = File;