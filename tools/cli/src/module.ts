import {logger} from "./logger";
import * as path from "path";

export type UserArray<T> = undefined | T | T[];

export interface CxxConfig {
    /**
     * Adds C++ compile definition (-D compiler option)
     */
    cpp_define: UserArray<string>;

    /**
     * Link C++ libraries
     */
    cpp_lib: UserArray<string>;

    /**
     * C++ source roots. Adds this directory as headers search path and detect all source files (translation units)
     * and adds them to compilation.
     *
     * Directory: used as path glob (basepath + var + "**|*.{default C++ file extensions}")
     * TODO: File: adds file to compilation
     * TODO: Glob pattern
     *
     * NOTE: Xcode generation use only Directory case and add folder to the project automatically adds Compile Sources
     */
    cpp: UserArray<string>;

    /**
     * Include directory: adds this directory to headers search path (-I compiler option) and allow to use that root
     * for `#include <header>` code
     */
    cpp_include: UserArray<string>;

    /**
     * Configure each C++ translation unit with compile flags explicitly
     * NOTE: currently supported only for XCode targets
     * TODO: files -> flags map pattern, better naming for CppFlags as well
     */
    cpp_flags: UserArray<CppFlags>;
}

export interface CppFlags {
    files?: string[];
    flags?: string;
}

type CppName =
    "cpp"
    | "cpp_include"
    | "cpp_flags"
    | "cpp_define"
    | "cpp_lib";
const cpp_names: CppName[] = [
    "cpp",
    "cpp_include",
    "cpp_flags",
    "cpp_define",
    "cpp_lib",
];


export interface AppleConfig extends CxxConfig {
    xcode_capability: UserArray<string>;
    xcode_framework: UserArray<string>;
    xcode_pod: UserArray<string>;
    xcode_plist: UserArray<any>;
    /**
     * files copied in Project which sould be added to project (path relative to xcode project's root)
     */
    xcode_file: UserArray<string>;

    xcode_projectPythonPostScript: UserArray<string>;
}

type AppleName =
    "xcode_capability"
    | "xcode_framework"
    | "xcode_pod"
    | "xcode_plist"
    | "xcode_file"
    | "xcode_projectPythonPostScript";
const apple_names: AppleName[] = [
    "xcode_capability",
    "xcode_framework",
    "xcode_pod",
    "xcode_plist",
    "xcode_file",
    "xcode_projectPythonPostScript"
];

export interface AndroidConfig extends CxxConfig {
    android_java: UserArray<string>;

    // LEGACY: option to include some native android services via "android interface declaration language"
    android_aidl: UserArray<string>;

    // app module gradle dependency string
    // TODO: should be more declarative object
    android_dependency: UserArray<string>;

    /**
     * root build.gradle buildScript dependencies
     */
    android_buildScriptDependency: UserArray<string>;

    /**
     * Patch :app build.gradle with `apply plugin: '{}'`
     * TODO: rename to android_gradlePlugin
     */
    android_gradleApplyPlugin: UserArray<string>;

    // add to the root of AndroidManifest.xml
    android_manifest: UserArray<string>;

    /**
     * Add <uses-permission> to the AndroidManifest.xml
     * `<uses-permission android:name="VALUE" />`
     * for example: `android.permission.VIBRATE`
     */
    android_permission: UserArray<string>;

    // add to the <application> xml node of AndroidManifest.xml
    android_manifestApplication: UserArray<string>;

    /**
     * Dictionaries to generate the final strings resource xml file:
     * `app/src/main/res/values/strings.xml`
     */
    android_strings: UserArray<string>;
}

type AndroidName =
    "android_java"
    | "android_aidl"
    | "android_dependency"
    | "android_buildScriptDependency"
    | "android_gradleApplyPlugin"
    | "android_gradleConfigRelease"
    | "android_manifest"
    | "android_permission"
    | "android_manifestApplication"
    | "android_strings";

const android_names: AndroidName[] = [
    "android_java",
    "android_aidl",
    "android_dependency",
    "android_buildScriptDependency",
    "android_gradleApplyPlugin",
    "android_gradleConfigRelease",
    "android_manifest",
    "android_permission",
    "android_manifestApplication",
    "android_strings",
];

export interface WasmConfig extends CxxConfig {
    // search all `**/*.js` in directories and link them as Emscripten JavaScript library (`--js-library` option)
    js: UserArray<string>;
    // search all `**/*.js` in directories and pre-concatenate them to the compiled JavaScript wrapper (`--pre-js` option)
    js_pre: UserArray<string>;
}

type JSName = "js" | "js_pre";
const js_names: JSName[] = ["js", "js_pre"];

export type VariableName = "assets" | CppName | AppleName | AndroidName | JSName;

export interface ModuleDef extends CxxConfig {
    name?: string;
    path?: string;
    assets?: UserArray<string>;

    android?: AndroidConfig;

    // shared options between macos and ios
    apple?: AppleConfig;
    macos?: AppleConfig;
    ios?: AppleConfig;

    web?: WasmConfig;
    windows?: CxxConfig;
    linux?: CxxConfig;
}

function validateFields(module: ModuleDef, data: any, scope: string, ...sets: string[][]) {
    if (data) {
        let errors = [];
        for (const key of Object.keys(data)) {
            let found = false;
            for (const set of sets) {
                if (set.indexOf(key) >= 0) {
                    found = true;
                }
            }
            if (!found) {
                errors.push(`\tKey "${key}" is not used`);
            }
        }
        if (errors.length > 0) {
            logger.error(`Module ${module.name} (file://${path.join(module.path, "ek.js")}) has invalid keys for "${scope}" scope:\n` +
                errors.join("\n"));
            logger.warn("Allowed keys:\n\t" + sets.map(p => "[" + p.join(", ") + "]").join("\n\t"));
        }
    }
}

export function validateModuleDef(module: ModuleDef) {
    validateFields(module, module, "top", ["name", "path"], ["assets"], ["android", "apple", "macos", "ios", "web", "windows", "linux"], cpp_names);
    validateFields(module, module.ios, "ios", apple_names, cpp_names);
    validateFields(module, module.macos, "macos", apple_names, cpp_names);
    validateFields(module, module.apple, "apple", apple_names, cpp_names);
    validateFields(module, module.android, "android", cpp_names, android_names);
    validateFields(module, module.web, "web", js_names, cpp_names);
    validateFields(module, module.windows, "windows", cpp_names);
    validateFields(module, module.linux, "linux", cpp_names);
}