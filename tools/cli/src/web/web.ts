import * as fs from "fs";
import {copyFileSync} from "fs";
import * as path from "path";
import {copyFolderRecursiveSync, executeAsync, isDir, isFile, makeDirs, withPath} from "../utils";
import {buildAssetsAsync} from "../assets";
import * as Mustache from 'mustache';
import {webBuildAppIconAsync} from "./webAppIcon";
import {collectLists, collectSourceFiles, collectSourceRootsAll} from "../collectSources";
import {Project} from "../project";
import {cmake} from "../cmake/build";
import {serve} from "./serve";
import {CMakeGenerateProject, CMakeGenerateTarget, cmakeLists} from "../cmake/generate";

function getEmscriptenSDKPath(): string {
    if (process.env.EMSDK) {
        return process.env.EMSDK;
    }
    return path.join(process.env.HOME, "dev/emsdk");
}

function renderCMakeFile(ctx, buildType): string {
    const cppSourceFiles = [];
    const cppExtensions = ["hpp", "hxx", "h", "cpp", "cxx", "c", "cc", "m", "mm"];
    const cppSourceRoots = collectSourceRootsAll(ctx, "cpp", "web", ".");
    for (const cppSourceRoot of cppSourceRoots) {
        collectSourceFiles(cppSourceRoot, cppExtensions, cppSourceFiles);
    }
    const cpp_include_path_list = collectSourceRootsAll(ctx, "cpp_include_path", "web", ".");

    const jsExtensions = ["js"];

    const jsLibraryFiles = [];
    const jsLibraryRoots = collectSourceRootsAll(ctx, "js", "web", ".");
    for (const jsLibraryRoot of jsLibraryRoots) {
        collectSourceFiles(jsLibraryRoot, jsExtensions, jsLibraryFiles);
    }

    const jsPreFiles = [];
    const jsPreRoots = collectSourceRootsAll(ctx, "pre_js", "web", ".");
    for (const jsPreRoot of jsPreRoots) {
        collectSourceFiles(jsPreRoot, jsExtensions, jsPreFiles);
    }

    const cppDefines = collectLists(ctx, "cppDefines", "web");
    const cppLibs = collectLists(ctx, "cppLibs", "web");

    const cmakeTarget: CMakeGenerateTarget = {
        type: "executable",
        libraryType: "static",
        name: ctx.name,
        sources: cppSourceFiles,
        includeDirectories: cppSourceRoots.concat(cpp_include_path_list),
        linkLibraries: cppLibs,
        linkOptions: [],
        compileOptions: ["-ffast-math", "-fno-exceptions", "-fno-rtti", "-Wall", "-Wextra"],
        compileDefinitions: cppDefines
    };

    const cmakeProject: CMakeGenerateProject = {
        cmakeVersion: "3.19",
        project: ctx.name,
        targets: [cmakeTarget],
        compileOptions: [],
        compileDefinitions: []
    };

    if (buildType === "Release") {
        cmakeTarget.linkOptions.push("-Oz", "-flto", "-g0");
        cmakeTarget.compileOptions.push("-Oz", "-flto", "-g0");
        cmakeTarget.compileDefinitions.push("NDEBUG");
    }

    for (let jsLibraryFile of jsLibraryFiles) {
        cmakeTarget.linkOptions.push(`SHELL:--js-library \${CMAKE_CURRENT_SOURCE_DIR}/${jsLibraryFile}`);
    }
    for (let jsPreFile of jsPreFiles) {
        cmakeProject.targets[0].linkOptions.push(`SHELL:--pre-js \${CMAKE_CURRENT_SOURCE_DIR}/${jsPreFile}`);
    }

    const emOptions: any = {
        ASSERTIONS: buildType === "Debug" ? 1 : 0,
        // SAFE_HEAP: 1,
        // STACK_OVERFLOW_CHECK: 2,
        // ALIASING_FUNCTION_POINTERS: 0,
        // MODULARIZE: 1,
        FETCH: 1,
        // WASM: 1,
        // WASM_ASYNC_COMPILATION: 1,
        DISABLE_EXCEPTION_CATCHING: 1,

        ALLOW_MEMORY_GROWTH: 1,

        // INITIAL_MEMORY: "128MB",
        // ALLOW_MEMORY_GROWTH: 0,

        FILESYSTEM: 0,
        INLINING_LIMIT: 1,
        //WASM_OBJECT_FILES: 0,

        // TODO: strange runtime DOM exception error with Release
        //STRICT: 1,
        ENVIRONMENT: "web",

        DYNAMIC_EXECUTION: 0,
        // AUTO_JS_LIBRARIES: 0,
        // AUTO_NATIVE_LIBRARIES: 0,
        AUTOLOAD_DYLIBS: 0,
        ALLOW_UNIMPLEMENTED_SYSCALLS: 0
    };

    cmakeTarget.linkOptions.push("--closure 1");

    for (let opt of Object.keys(emOptions)) {
        cmakeTarget.linkOptions.push(`-s${opt}=${emOptions[opt]}`);
    }
    return cmakeLists(cmakeProject);
}

function getCMakeBuildDir(buildType: string) {
    return "cmake-build-" + buildType.toLowerCase();
}

async function buildProject(ctx, buildType) {
    const platform_proj_name = ctx.name + "-" + ctx.current_target; // "projectName-web"
    const dest_dir = path.resolve(process.cwd(), "export");
    const output_path = path.join(dest_dir, platform_proj_name);
    const cmakeBuildDir = getCMakeBuildDir(buildType);

    if (!isDir(output_path)) {
        await fs.promises.mkdir(output_path, {recursive: true});
    } else {
        const outputJsPath = path.join(output_path, cmakeBuildDir, ctx.name + ".js");
        if (isFile(outputJsPath)) {
            // just in case of changed any js linking dependencies :(
            await fs.promises.rm(outputJsPath);
        }
    }

    {
        const cmakeFile = withPath(output_path, () => renderCMakeFile(ctx, buildType));
        await fs.promises.writeFile(path.join(output_path, "CMakeLists.txt"), cmakeFile);
    }

    {
        const EMSDK_PATH = getEmscriptenSDKPath();
        const EMSDK_CMAKE_TOOLCHAIN = path.join(EMSDK_PATH, "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake");
        await cmake(cmakeBuildDir, {
            toolchain: EMSDK_CMAKE_TOOLCHAIN,
            buildType: buildType,
            ccache: false,
            workingDir: output_path
        });
    }
}

/*** HTML ***/
export async function export_web(ctx: Project): Promise<void> {
    const timestamp = Date.now();

    if (ctx.html.og) {
        if (!ctx.html.og.title && ctx.title) {
            ctx.html.og.title = ctx.title;
        }
        if (!ctx.html.og.description && ctx.desc) {
            ctx.html.og.description = ctx.desc;
        }
    }
    const outputDir = path.join(ctx.path.CURRENT_PROJECT_DIR, "export/web");
    makeDirs(outputDir);

    function tpl(from, to) {
        const tpl_text = fs.readFileSync(path.join(ctx.path.templates, from), "utf8");
        fs.writeFileSync(path.join(outputDir, to), Mustache.render(tpl_text, ctx), "utf8");
    }

    function file(from, to) {
        fs.copyFileSync(
            path.join(ctx.path.templates, from),
            path.join(outputDir, to)
        );
    }

    const buildType = ctx.args.indexOf("--debug") >= 0 ? "Debug" : "Release";
    const buildTask = buildProject(ctx, buildType);
    const assetsTask = buildAssetsAsync(ctx);

    const webManifest = JSON.parse(fs.readFileSync(path.join(ctx.path.templates, "web/manifest.json"), "utf8"));
    webManifest.name = ctx.title;
    webManifest.short_name = ctx.name;
    webManifest.description = ctx.desc;
    webManifest.start_url = (ctx.pwa_url ?? "") + "/index.html";
    if (ctx.web?.applications != null) {
        webManifest.related_applications = ctx.web?.applications;
    }

    fs.writeFileSync(path.join(outputDir, "manifest.json"), JSON.stringify(webManifest), "utf8");
    const iconsTask = webBuildAppIconAsync(ctx, webManifest.icons, outputDir);

    tpl("web/index.html.mustache", "index.html");
    tpl("web/sw.js", "sw.js");
    file("web/pwacompat.min.js", "pwacompat.min.js");

    try {
        await assetsTask;
    } catch (e) {
        console.error("assets export failed", e);
        throw e;
    }
    copyFolderRecursiveSync("export/contents/assets", "export/web/assets");

    try {
        await buildTask;
    } catch (e) {
        console.error("build failed", e);
        throw e;
    }
    const cmakeBuildDir = getCMakeBuildDir(buildType);
    const projectDir = path.join(ctx.path.CURRENT_PROJECT_DIR, "export", ctx.name + "-" + ctx.current_target);
    copyFileSync(path.join(projectDir, cmakeBuildDir, ctx.name + ".js"), path.join(outputDir, ctx.name + ".js"));
    copyFileSync(path.join(projectDir, cmakeBuildDir, ctx.name + ".wasm"), path.join(outputDir, ctx.name + ".wasm"));

    try {
        await iconsTask;
    } catch (e) {
        console.error("icons export failed", e);
        throw e;
    }

    console.info("Web export completed");
    console.info("Time:", (Date.now() - timestamp) / 1000, "sec");

    if (ctx.options.deploy != null) {
        // always deploy just to the default firebase hosting
        console.info("Publish Web beta to Firebase host");
        const args = [];
        let token = process.env.FIREBASE_TOKEN;
        if (!token && ctx.web.firebaseToken) {
            try {
                if (fs.existsSync(ctx.web.firebaseToken)) {
                    token = fs.readFileSync(ctx.web.firebaseToken, 'utf-8');
                } else {
                    console.error(`Firebase Token file path not found`);
                }
            } catch {
                console.error(`Cannot read Firebase Token`);
            }
        }
        if (token) {
            args.push("--token", token);
        } else {
            console.warn("No Firebase Token. Trying deploy with local firebase auth");
        }
        await executeAsync("firebase", [
            "deploy",
            "--only", "hosting",
            ...args
        ]);
    }

    if (ctx.options.run != null) {
        await serve(outputDir);
    }
}