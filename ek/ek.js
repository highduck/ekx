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

function collect_source_files(search_path, out_list = []) {
    search_files("**/*.h", search_path, out_list);
    search_files("**/*.hpp", search_path, out_list);
    search_files("**/*.cpp", search_path, out_list);
    return out_list;
}

function mod_cmake_lists(ctx) {
    const cmake_path = "app/CMakeLists.txt";
    const search_list = [
        "../../src",
        path.join(ctx.path.EKX_ROOT, "ecxx/src"),
        path.join(ctx.path.EKX_ROOT, "core/src"),
        path.join(ctx.path.EKX_ROOT, "ek/platforms/android"),
        path.join(ctx.path.EKX_ROOT, "ek/src"),
        path.join(ctx.path.EKX_ROOT, "scenex/src")
    ];

    const src_files = [];
    collect_source_files("../../src", src_files);
    collect_source_files(path.join(ctx.path.EKX_ROOT, "ecxx/src"), src_files);
    collect_source_files(path.join(ctx.path.EKX_ROOT, "core/src"), src_files);
    collect_source_files(path.join(ctx.path.EKX_ROOT, "ek/src"), src_files);
    collect_source_files(path.join(ctx.path.EKX_ROOT, "ek/platforms/android"), src_files);
    collect_source_files(path.join(ctx.path.EKX_ROOT, "scenex/src"), src_files);

    for (let i = 0; i < search_list.length; ++i) {
        const p = search_list[i];
        if (p.length > 0 && p[0] !== '/') {
            search_list[i] = path.join("..", p);
        }
    }

    for (let i = 0; i < src_files.length; ++i) {
        const p = src_files[i];
        if (p.length > 0 && p[0] !== '/') {
            src_files[i] = path.join("..", p);
        }
    }

    replace_in_file(cmake_path, {
        "#-SOURCES-#": src_files.join("\n\t\t"),
        "#-SEARCH_ROOTS-#": search_list.join("\n\t\t")
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
        execute("python3", ["xcode-project-ios.py", platform_proj_name, ctx.ios.application_id, ctx.path.EKX_ROOT]);

        console.info("Prepare PodFile");
        replace_in_file("Podfile", {
            "template-ios": platform_proj_name
        });

        console.info("Install Pods");
        execute("pod", ["install"]);

        /// POST MOD PROJECT
        execute("python3", ["xcode-project-ios-post.py",
            platform_proj_name, ctx.ios.application_id]);
    }
    process.chdir(cwd);

    const workspace_path = path.join(dest_path, platform_proj_name + ".xcworkspace");
    // execute("open", [dest_path]);
//    execute("open " + workspace_path.str());
    execute("xcodebuild", [
        "-workspace", workspace_path,
        "-scheme", platform_proj_name,
        "-configuration", "Release"
    ]);
}


class File {
    constructor(ctx) {
        ctx.android = {};
        ctx.ios = {};
        ctx.html = {};

        console.log("=== EK PROJECT ===");
        console.log("Current Target:", ctx.current_target);

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

        console.log("Loading EK platforms project");
        if (ctx.current_target === "web") {
            ctx.build_steps.push(() => {
                tpl("templates/web/index.html.mustache", "index.html");
                tpl("templates/web/manifest.json.mustache", "manifest.webmanifest");
                tpl("templates/web/sw.js.mustache", "sw.js");
                file("templates/web/howler.core.min.js", "howler.core.min.js");
                file("templates/web/pwacompat.min.js", "pwacompat.min.js");
                file("platforms/web/audiomini.js", "audiomini.js");
            });

            // if (!project.html.deploy_dir.empty()) {
            //     copy_file(output_dir / "pwacompat.min.js",
            //         project.html.deploy_dir / "pwacompat.min.js");
            //
            //     copy_file(output_dir / "howler.core.min.js",
            //         project.html.deploy_dir / "howler.core.min.js");
            //
            //     copy_file(output_dir / "manifest.webmanifest",
            //         project.html.deploy_dir / "manifest.webmanifest");
            //     copy_file(output_dir / "sw.js", project.html.deploy_dir / "sw.js");
            //     copy_file(output_dir / "index.html", project.html.deploy_dir / "index.html");
            //     copy_file(output_dir / (project.binary_name + ".wasm"),
            //         project.html.deploy_dir / (project.binary_name + ".wasm"));
            //     copy_file(output_dir / (project.binary_name + ".js"),
            //         project.html.deploy_dir / (project.binary_name + ".js"));
            //     copy_file(output_dir / (project.binary_name + ".data"),
            //         project.html.deploy_dir / (project.binary_name + ".data"));
            //
            //     // TODO: icons / assets
            // }
        }

        if (ctx.current_target === "android") {
            ctx.build_steps.push(() => {
                export_android(ctx);
            });
        }

        if (ctx.current_target === "ios") {
            ctx.build_steps.push(() => {
                export_ios(ctx);
            });
        }
    }
}

module.exports = File;