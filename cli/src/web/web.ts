import * as fs from "fs";
import * as path from "path";
import {copyFolderRecursiveSync, execute, isDir, substituteAll} from "../utils";
import {buildAssets} from "../assets";
import * as Mustache from 'mustache';
import {webBuildAppIcon} from "./webAppIcon";
import {collectSourceFiles, collectSourceRootsAll} from "../collectSources";
import {copyFileSync} from "fs";

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

    return substituteAll(cmakeListContents, {
        "TEMPLATE_PROJECT_NAME": ctx.name,
        "#-SOURCES-#": cppSourceFiles.join("\n\t\t"),
        "#-SEARCH_ROOTS-#": cppSourceRoots.join("\n\t\t"),
        "#-LINK_OPTIONS-#": jsSourceFiles.map(s => `--js-library \${CMAKE_CURRENT_SOURCE_DIR}/${s}`).join("\n\t\t"),
    });
}

function buildProject(ctx) {
    const buildType = "Release";
    const platform_proj_name = ctx.name + "-" + ctx.current_target; // "projectName-web"
    const dest_dir = "export";
    const dest_path = path.join(dest_dir, platform_proj_name);

    const output_path = path.join(ctx.path.CURRENT_PROJECT_DIR, dest_path);
    if (!isDir(output_path)) {
        fs.mkdirSync(output_path, {recursive: true});
    }

    {
        const cwd = process.cwd();
        process.chdir(output_path);
        let cmakeFile = fs.readFileSync(path.join(ctx.path.templates, "project-web/CMakeLists.txt"), "utf8");
        cmakeFile = renderCMakeFile(ctx, cmakeFile);
        fs.writeFileSync(path.join(output_path, "CMakeLists.txt"), cmakeFile);
        process.chdir(cwd);
    }

    {
        const EMSDK_PATH = "/Users/ilyak/dev/emsdk";
        const EMSDK_CMAKE_TOOLCHAIN = path.join(EMSDK_PATH, "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake");
        const args = ["-GNinja", `-DCMAKE_TOOLCHAIN_FILE=${EMSDK_CMAKE_TOOLCHAIN}`, `-DCMAKE_BUILD_TYPE=${buildType}`]; // -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
        const buildDir = path.join(output_path, `cmake-build-${buildType.toLowerCase()}`);
        if (!isDir(buildDir)) {
            fs.mkdirSync(buildDir);
        }

        const cwd = process.cwd();
        process.chdir(buildDir);
        {
            execute("cmake", [".."].concat(args));
            execute("ninja", []);
        }
        process.chdir(cwd);
    }
}

/*** HTML ***/
export function export_web(ctx) {
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

    buildAssets(ctx);
    webBuildAppIcon(ctx, path.join(output_dir, "icons"));

    buildProject(ctx);

    tpl("web/index.html.mustache", "index.html");
    tpl("web/manifest.json.mustache", "manifest.webmanifest");
    tpl("web/sw.js.mustache", "sw.js");
    file("web/pwacompat.min.js", "pwacompat.min.js");

    copyFolderRecursiveSync("export/contents/assets", "export/web/assets");

    const projectDir = path.join(ctx.path.CURRENT_PROJECT_DIR, "export", ctx.name + "-" + ctx.current_target);
    copyFileSync(path.join(projectDir, "build", ctx.name + ".js"), path.join(output_dir, ctx.name + ".js"));
    copyFileSync(path.join(projectDir, "build", ctx.name + ".wasm"), path.join(output_dir, ctx.name + ".wasm"));
}