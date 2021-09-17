import * as fs from "fs";
import {copyFileSync} from "fs";
import * as path from "path";
import {executeAsync, isDir, makeDirs, replaceInFile, withPath} from "../utils";
import {buildAssetPackAsync} from "../assets";
import * as Mustache from 'mustache';
import {webBuildAppIconAsync} from "./webAppIcon";
import {collectSourceFiles, collectSourceRootsAll, collectStrings} from "../collectSources";
import {Project} from "../project";
import {build as buildCMake, BuildResult, CMakeGenerateProject, CMakeGenerateTarget, cmakeLists} from "cmake-build";
import {serve} from "./serve";
import {logger} from "../logger";
import * as glob from "glob";

function renderCMakeFile(ctx, buildType): string {
    const platforms = ["web"];
    const cppSourceFiles = [];
    const cppExtensions = ["hpp", "hxx", "h", "cpp", "cxx", "c", "cc", "m", "mm"];
    const cppSourceRoots = collectSourceRootsAll(ctx, "cpp", platforms, ".");
    for (const cppSourceRoot of cppSourceRoots) {
        collectSourceFiles(cppSourceRoot, cppExtensions, cppSourceFiles);
    }
    const cpp_include = collectSourceRootsAll(ctx, "cpp_include", platforms, ".");

    const jsExtensions = ["js"];

    const jsLibraryFiles = [];
    const js = collectSourceRootsAll(ctx, "js", platforms, ".");
    for (const jsLibraryRoot of js) {
        collectSourceFiles(jsLibraryRoot, jsExtensions, jsLibraryFiles);
    }

    const jsPreFiles = [];
    const js_pre = collectSourceRootsAll(ctx, "js_pre", platforms, ".");
    for (const jsPreRoot of js_pre) {
        collectSourceFiles(jsPreRoot, jsExtensions, jsPreFiles);
    }

    const cpp_define = collectStrings(ctx, "cpp_define", platforms, false);
    const cpp_lib = collectStrings(ctx, "cpp_lib", platforms, false);

    const cmakeTarget: CMakeGenerateTarget = {
        type: "executable",
        libraryType: "static",
        name: ctx.name,
        sources: cppSourceFiles,
        includeDirectories: cppSourceRoots.concat(cpp_include),
        linkLibraries: cpp_lib,
        linkOptions: [],
        compileOptions: ["-ffast-math", "-fno-exceptions", "-fno-rtti", "-Wall", "-Wextra"],
        compileDefinitions: cpp_define
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
        if (1) {
            cmakeTarget.linkOptions.push("-g0");
            cmakeTarget.compileOptions.push("-g0");
        } else {
            cmakeTarget.linkOptions.push("-gsource-map");
            cmakeTarget.compileOptions.push("-g");
        }
        cmakeTarget.compileDefinitions.push("NDEBUG");
    }

    if (buildType === "Debug") {
        cmakeTarget.linkOptions.push("-Oz", "-gsource-map");
        cmakeTarget.compileOptions.push("-Oz", "-g");
    }

    for (let jsLibraryFile of jsLibraryFiles) {
        cmakeTarget.linkOptions.push(`SHELL:--js-library \${CMAKE_CURRENT_SOURCE_DIR}/${jsLibraryFile}`);
    }
    for (let jsPreFile of jsPreFiles) {
        cmakeProject.targets[0].linkOptions.push(`SHELL:--pre-js \${CMAKE_CURRENT_SOURCE_DIR}/${jsPreFile}`);
    }

    const emOptions: any = {
        ASSERTIONS: buildType === "Debug" ? 2 : 0,
        SAFE_HEAP: buildType === "Debug" ? 1 : 0,
        DEMANGLE_SUPPORT: buildType === "Debug" ? 1 : 0,

        // TODO: strange runtime DOM exception error with Release
        STRICT: 1,
        MINIMAL_RUNTIME: 1,
        //MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION: 1,
        // MODULARIZE: 1,
        SUPPORT_ERRNO: 0,

        // STACK_OVERFLOW_CHECK: 2,
        // ALIASING_FUNCTION_POINTERS: 0,

        FETCH: 0,
        // WASM: 1,
        // WASM_ASYNC_COMPILATION: 1,
        DISABLE_EXCEPTION_CATCHING: 1,

        ALLOW_MEMORY_GROWTH: 1,

        // INITIAL_MEMORY: "128MB",
        // ALLOW_MEMORY_GROWTH: 0,

        FILESYSTEM: 0,
        INLINING_LIMIT: 1,
        //WASM_OBJECT_FILES: 0,

        ENVIRONMENT: "web",

        DYNAMIC_EXECUTION: 0,
        // AUTO_JS_LIBRARIES: 0,
        // AUTO_NATIVE_LIBRARIES: 0,
        AUTOLOAD_DYLIBS: 0,
        ALLOW_UNIMPLEMENTED_SYSCALLS: 0
    };

    if (buildType === "Release") {
        cmakeTarget.linkOptions.push("--closure 1");
    }

    for (let opt of Object.keys(emOptions)) {
        cmakeTarget.linkOptions.push(`-s${opt}=${emOptions[opt]}`);
    }
    return cmakeLists(cmakeProject);
}

async function buildProject(ctx, buildType) {
    const platform_proj_name = ctx.name + "-" + ctx.current_target; // "projectName-web"
    const dest_dir = path.resolve(process.cwd(), "export");
    const output_path = path.join(dest_dir, platform_proj_name);

    if (!isDir(output_path)) {
        await fs.promises.mkdir(output_path, {recursive: true});
    }

    const cmakeFile = withPath(output_path, () => renderCMakeFile(ctx, buildType));

    // if you need to look into generated html by Emscripten
    // cmakeFile += "\n\n" + `set(CMAKE_EXECUTABLE_SUFFIX ".html")`

    await fs.promises.writeFile(path.join(output_path, "CMakeLists.txt"), cmakeFile);

    return await buildCMake({
        os: "web",
        buildType: buildType,
        workingDir: output_path
    });
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
    const assetsTask = buildAssetPackAsync(ctx, path.join(outputDir, "assets"));

    const webManifest = JSON.parse(fs.readFileSync(path.join(ctx.path.templates, "web/manifest.json"), "utf8"));
    webManifest.name = ctx.title;
    webManifest.short_name = ctx.name;
    webManifest.version = ctx.version_name;
    webManifest.version_code = ctx.version_code;
    webManifest.description = ctx.desc;
    webManifest.start_url = "./index.html";
    if (ctx.web?.applications != null) {
        webManifest.related_applications = ctx.web?.applications;
    }

    fs.writeFileSync(path.join(outputDir, "manifest.json"), JSON.stringify(webManifest), "utf8");
    const iconsTask = webBuildAppIconAsync(ctx, webManifest.icons, outputDir);

    tpl("web/index.html.mustache", "index.html");
    file("web/pwacompat.min.js", "pwacompat.min.js");
    tpl("web/sw.js", "sw.js");

    try {
        await assetsTask;
    } catch (e) {
        logger.error("assets export failed", e);
        throw e;
    }

    const assetDirFiles = glob.sync(path.join(outputDir, "assets/**/*"));
    const assetsList = assetDirFiles.map(p => `"${path.relative(outputDir, p)}"`).join(",\n");
    replaceInFile(path.join(outputDir, "sw.js"), {
        "var contentToCache = [];": `var contentToCache = [
        'index.html',
        'pwacompat.min.js',
        '${ctx.name}.js',
        '${ctx.name}.wasm',
        ${assetsList}
];`
    });

    let buildResult!: BuildResult;
    try {
        buildResult = await buildTask;
    } catch (e) {
        logger.error("build failed", e);
        throw e;
    }
    //const projectDir = path.join(ctx.path.CURRENT_PROJECT_DIR, "export", ctx.name + "-" + ctx.current_target);
    copyFileSync(path.join(buildResult.buildDir, ctx.name + ".js"), path.join(outputDir, ctx.name + ".js"));
    copyFileSync(path.join(buildResult.buildDir, ctx.name + ".wasm"), path.join(outputDir, ctx.name + ".wasm"));
    //if (buildType === "Debug") {
    try {
        copyFileSync(path.join(buildResult.buildDir, ctx.name + ".wasm.map"), path.join(outputDir, ctx.name + ".wasm.map"));
    } catch {
    }
    //}

    try {
        await iconsTask;
    } catch (e) {
        logger.error("icons export failed", e);
        throw e;
    }

    logger.info("Web export completed");
    logger.info("Time:", (Date.now() - timestamp) / 1000, "sec");

    if (ctx.options.deploy != null) {
        // always deploy just to the default firebase hosting
        logger.info("Publish Web beta to Firebase host");
        const args = [];
        let token = process.env.FIREBASE_TOKEN;
        if (!token && ctx.web.firebaseToken) {
            try {
                if (fs.existsSync(ctx.web.firebaseToken)) {
                    token = fs.readFileSync(ctx.web.firebaseToken, 'utf-8');
                } else {
                    logger.error(`Firebase Token file path not found`);
                }
            } catch {
                logger.error(`Cannot read Firebase Token`);
            }
        }
        if (token) {
            args.push("--token", token);
        } else {
            logger.warn("No Firebase Token. Trying deploy with local firebase auth");
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