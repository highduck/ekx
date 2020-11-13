import {
    copy_file,
    copyFolderRecursiveSync,
    deleteFolderRecursive,
    execute,
    is_dir,
    is_file,
    make_dirs,
    optimize_png_glob,
    read_text,
    replace_all,
    replace_in_file,
    write_text
} from "../utils";
import {XmlDocument} from 'xmldoc';
import * as path from "path";
import {ekc_export_assets_lazy, ekc_export_market} from "../assets";
import {collect_source_files, collect_src_roots_all} from "../collectSources";
import {copySigningKeys, printSigningConfigs} from "./signing";

function open_android_project(android_project_path) {
    execute("open", ["-a", "/Applications/Android Studio.app", android_project_path]);
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
    const res_strings_path = "app/src/main/res/values/strings.xml";
    const xml_text = read_text(res_strings_path);
    const doc = new XmlDocument(xml_text);
    // console.trace(doc.firstChild);
    // console.trace(doc.childrenNamed("string"));
    doc.eachChild((child) => {
        if (child.name === "string") {
            const textNode = child.firstChild.type == 'text' ? child.firstChild : null;
            if (textNode) {
                switch (child.attr.name) {
                    case "app_name":
                        // seems xmldoc is not enough
                        child.val = ctx.title;
                        textNode.text = ctx.title;
                        break;
                    case "package_name":
                        textNode.text = ctx.android.application_id;
                        break;
                    case "gs_app_id":
                        textNode.text = ctx.android.game_services_id;
                        break;
                    case "admob_app_id":
                        textNode.text = ctx.android.admob_app_id;
                        break;
                }
            }
        }
    });

    write_text(res_strings_path, doc.toString());
}


function mod_cmake_lists(ctx) {
    const cmake_path = "CMakeLists.txt";
    const src_files = [];
    const ext_list = ["hpp", "hxx", "h", "cpp", "cxx", "c"];

    const source_dir_list = collect_src_roots_all(ctx, "cpp", "android", ".");

    for (const source_dir of source_dir_list) {
        collect_source_files(source_dir, ext_list, src_files);
    }

    replace_in_file(cmake_path, {
        "#-SOURCES-#": src_files.join("\n\t\t"),
        "#-SEARCH_ROOTS-#": source_dir_list.join("\n\t\t")
    });
}


export function export_android(ctx) {

    ekc_export_assets_lazy(ctx);
    if (!is_dir("export/android/res")) {
        ekc_export_market(ctx, "android", "export/android/res");
        optimize_png_glob("export/android/res/**/*.png");
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

    copyFolderRecursiveSync(path.join(ctx.path.templates, `template-${platform_target}`), dest_path);
    const base_path = "../..";

    const cwd = process.cwd();
    process.chdir(dest_path);
    {
        const java_roots =
            collect_src_roots_all(ctx, "java", "android", "app")
                .map((p) => `'${p}'`);

        const aidl_roots =
            collect_src_roots_all(ctx, "aidl", "android", "app")
                .map((p) => `'${p}'`);

        const assets_roots =
            collect_src_roots_all(ctx, "assets", "android", "app")
                .map((p) => `'${p}'`);

        const source_sets = [
            `main.java.srcDirs += [${java_roots.join(", ")}]`,
            `main.aidl.srcDirs += [${aidl_roots.join(", ")}]`,
            `main.assets.srcDirs += [${assets_roots.join(", ")}]`
        ];

        replace_in_file("app/build.gradle", {
            'com.eliasku.template_android': ctx.android.application_id,
            'versionCode 1 // AUTO': `versionCode ${ctx.version_code} // AUTO`,
            'versionName "1.0" // AUTO': `versionName "${ctx.version_name}" // AUTO`,
            '// TEMPLATE_SOURCE_SETS': source_sets.join("\n\t\t"),
            '// TEMPLATE_DEPENDENCIES': ctx.build.android.dependencies.join("\n\t"),
            '// ${SIGNING_CONFIGS}': printSigningConfigs(ctx.android.keystore),
        });

        copyFolderRecursiveSync(path.join(base_path, "export/android/res"), "app/src/main/res");

        mod_main_class(ctx.android.package_id);
        mod_android_manifest(ctx);
        mod_strings(ctx);
        mod_cmake_lists(ctx);
        copy_google_services_config_android();
        copySigningKeys(ctx.android.keystore);
    }
    process.chdir(cwd);
    open_android_project(dest_path);
}
