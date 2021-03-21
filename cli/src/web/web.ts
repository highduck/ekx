import * as fs from "fs";
import * as path from "path";
import {copyFolderRecursiveSync, executeAsync, isDir, isFile, substituteAll, withPath} from "../utils";
import {buildAssetsAsync} from "../assets";
import * as Mustache from 'mustache';
import {webBuildAppIconAsync} from "./webAppIcon";
import {collectSourceFiles, collectSourceRootsAll} from "../collectSources";
import {Project} from "../project";
import {copyFileSync} from "fs";

function getEmscriptenSDKPath():string {
    if(process.env.EMSDK) {
        return process.env.EMSDK;
    }
    return path.join(process.env.HOME, "dev/emsdk");
}

function renderCMakeFile(ctx, cmakeListContents: string): string {
    const cppSourceFiles = [];
    const cppExtensions = ["hpp", "hxx", "h", "cpp", "cxx", "c"];
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

    return substituteAll(cmakeListContents, {
        "TEMPLATE_PROJECT_NAME": ctx.name,
        "#-SOURCES-#": cppSourceFiles.join("\n\t\t"),
        "#-SEARCH_ROOTS-#": cppSourceRoots.join("\n\t\t"),
        "#-LINK_OPTIONS-#": jsSourceFiles.map(s => `--js-library \${CMAKE_CURRENT_SOURCE_DIR}/${s}`).join("\n\t\t"),
        "#{{{CMAKE_CODE}}}": cmakeAdditionalDependencies
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
        // -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
        await executeAsync("cmake", [
            ".",
            "-B", cmakeBuildDir,
            "-GNinja",
            `-DCMAKE_TOOLCHAIN_FILE=${EMSDK_CMAKE_TOOLCHAIN}`,
            `-DCMAKE_BUILD_TYPE=${buildType}`
        ], {workingDir: output_path});
        await executeAsync("cmake", [
            "--build", cmakeBuildDir
        ], {workingDir: output_path});
    }
}

/*** HTML ***/
export async function export_web(ctx: Project) {
    const timestamp = Date.now();

    const output_dir = ctx.path.CURRENT_PROJECT_DIR + "/export/web";

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
    const iconsTask = webBuildAppIconAsync(ctx, path.join(output_dir, "icons"));

    tpl("web/index.html.mustache", "index.html");
    tpl("web/manifest.json.mustache", "manifest.webmanifest");
    tpl("web/sw.js.mustache", "sw.js");
    file("web/pwacompat.min.js", "pwacompat.min.js");

    await assetsTask;
    copyFolderRecursiveSync("export/contents/assets", "export/web/assets");

    await buildTask;
    const cmakeBuildDir = getCMakeBuildDir(buildType);
    const projectDir = path.join(ctx.path.CURRENT_PROJECT_DIR, "export", ctx.name + "-" + ctx.current_target);
    copyFileSync(path.join(projectDir, cmakeBuildDir, ctx.name + ".js"), path.join(output_dir, ctx.name + ".js"));
    copyFileSync(path.join(projectDir, cmakeBuildDir, ctx.name + ".wasm"), path.join(output_dir, ctx.name + ".wasm"));

    await iconsTask;

    console.info("Web export completed");
    console.info("Time:", (Date.now() - timestamp) / 1000, "sec");

    if (ctx.options.deployBeta) {
        console.info("Publish Web beta to Firebase host");
        const args = [];
        let token = process.env.FIREBASE_TOKEN;
        if(!token && ctx.web.firebaseToken) {
            try {
                if(fs.existsSync(ctx.web.firebaseToken)) {
                    token = fs.readFileSync(ctx.web.firebaseToken, 'utf-8');
                }
                else {
                    console.error(`Firebase Token file path not found`);
                }
            }
            catch {
                console.error(`Cannot read Firebase Token`);
            }
        }
        if (token) {
            args.push("--token", token);
        }
        else {
            console.warn("No Firebase Token. Trying deploy with local firebase auth");
        }
        await executeAsync("firebase", [
            "deploy",
            "--only", "hosting",
            ...args
        ]);
    }
}