import * as fs from "fs";
import {copyFileSync} from "fs";
import * as path from "path";
import {
    copyFolderRecursiveSync,
    executeAsync,
    isDir,
    isFile,
    makeDirs,
    replaceInFile,
    substituteAll,
    withPath
} from "../utils";
import {buildAssetsAsync} from "../assets";
import * as Mustache from 'mustache';
import {webBuildAppIconAsync} from "./webAppIcon";
import {collectCompileDefines, collectSourceFiles, collectSourceRootsAll} from "../collectSources";
import {Project} from "../project";
import {cmake} from "../cmake";

function getEmscriptenSDKPath(): string {
    if (process.env.EMSDK) {
        return process.env.EMSDK;
    }
    return path.join(process.env.HOME, "dev/emsdk");
}

function renderCMakeFile(ctx, cmakeListContents: string): string {
    const cppSourceFiles = [];
    const cppExtensions = ["hpp", "hxx", "h", "cpp", "cxx", "c", "cc", "m", "mm"];
    const cppSourceRoots = collectSourceRootsAll(ctx, "cpp", "web", ".");
    for (const cppSourceRoot of cppSourceRoots) {
        collectSourceFiles(cppSourceRoot, cppExtensions, cppSourceFiles);
    }

    const jsSourceFiles = [];
    const jsExtensions = ["js"];
    const jsSourceRoots = collectSourceRootsAll(ctx, "js", "web", ".");
    for (const jsSourceRoot of jsSourceRoots) {
        collectSourceFiles(jsSourceRoot, jsExtensions, jsSourceFiles);
    }

    let cmakeAdditionalDependencies = "";
    if (jsSourceFiles.length > 0) {
        // const jsDepsTargetName = "main-js-deps";
        // cmakeAdditionalDependencies = `
        // add_custom_target(${jsDepsTargetName} DEPENDS ${jsSourceFiles.join(" ")})
        // set_source_files_properties(${jsSourceFiles.join(" ")} PROPERTIES GENERATED TRUE)
        // add_dependencies(\${PROJECT_NAME} ${jsDepsTargetName})`;
    }

    const compileDefines = collectCompileDefines(ctx, "cppDefines", "android");
    const cmakeCompileDefines = "target_compile_definitions(${PROJECT_NAME}\n" +
        compileDefines.map((x) => `\t\tPUBLIC ${x}`).join("\n") + "\n)";

    return substituteAll(cmakeListContents, {
        "TEMPLATE_PROJECT_NAME": ctx.name,
        "#-SOURCES-#": cppSourceFiles.join("\n\t\t"),
        "#-SEARCH_ROOTS-#": cppSourceRoots.join("\n\t\t"),
        "#-LINK_OPTIONS-#": jsSourceFiles.map(s => `--js-library \${CMAKE_CURRENT_SOURCE_DIR}/${s}`).join("\n\t\t"),
        "#{{{CMAKE_CODE}}}": cmakeAdditionalDependencies,
        "#-CPP_DEFINES-#": cmakeCompileDefines
    });
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
        let cmakeFile = await fs.promises.readFile(path.join(ctx.path.templates, "project-web/CMakeLists.txt"), "utf8");
        withPath(output_path, () => {
            cmakeFile = renderCMakeFile(ctx, cmakeFile);
        });
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
    const output_dir = path.join(ctx.path.CURRENT_PROJECT_DIR, "export/web");
    makeDirs(output_dir);

    function tpl(from, to) {
        const tpl_text = fs.readFileSync(path.join(ctx.path.templates, from), "utf8");
        fs.writeFileSync(path.join(output_dir, to), Mustache.render(tpl_text, ctx), "utf8");
    }

    function file(from, to) {
        fs.copyFileSync(
            path.join(ctx.path.templates, from),
            path.join(output_dir, to)
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

    fs.writeFileSync(path.join(output_dir, "manifest.json"), JSON.stringify(webManifest), "utf8");
    const iconsTask = webBuildAppIconAsync(ctx, webManifest.icons, output_dir);

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
    copyFileSync(path.join(projectDir, cmakeBuildDir, ctx.name + ".js"), path.join(output_dir, ctx.name + ".js"));
    copyFileSync(path.join(projectDir, cmakeBuildDir, ctx.name + ".wasm"), path.join(output_dir, ctx.name + ".wasm"));

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
}