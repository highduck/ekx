import * as fs from "fs";
import * as path from "path";
import Mustache from "mustache";
import * as esbuild from "esbuild";
import {replaceInFile} from "../utils.js";
import {buildAssetPackAsync} from "../assets.js";
import {Project} from "../project.js";
import {BuildResult} from "../../cmake/mod.js";
import {serve} from "./serve.js";
import {logger} from "../logger.js";
import {buildWasm} from "./buildWasm.js";
import {deployFirebaseHosting} from "./deployFirebaseHosting.js";
import {buildAppIconAsync} from "../appicon/appicon.js";
import {collectSourceRootsAll} from "../collectSources.js";
import {ensureDirSync, expandGlobSync, readTextFileSync, writeTextFileSync} from "../../utils/utils.js";

/*** HTML ***/
export async function export_web(ctx: Project): Promise<void> {
    const timestamp = Date.now();

    ctx.generateNativeBuildInfo();

    const og = ctx.web.og;
    if (og) {
        if (!og.title && ctx.title) {
            og.title = ctx.title;
        }
        if (!og.description && ctx.desc) {
            og.description = ctx.desc;
        }
    }
    const outputDir = path.join(ctx.projectPath, "export/web");
    ensureDirSync(outputDir);

    function render(str: string): string {
        const renderParameters = {
            name: ctx.name,
            title: ctx.title,
            desc: ctx.desc,
            version_code: ctx.version.buildNumber(),
            version_name: ctx.version.name(),
            html: {
                og,
                background_color: ctx.web.background_color,
                text_color: ctx.web.text_color,
            },
            bodyCode: `${ctx.web.bodyHTML.join("\n")}
<script>${ctx.web.bodyScript.join("\n")}</script>`,
            headCode: ctx.web.headCode.join("\n")
        };
        return Mustache.render(str, renderParameters);
    }

    function tpl(from: string, to: string) {
        const tplContent = readTextFileSync(path.join(ctx.sdk.templates, from));
        writeTextFileSync(path.join(outputDir, to), render(tplContent));
    }

    function file(from: string, to: string) {
        fs.copyFileSync(
            path.join(ctx.sdk.templates, from),
            path.join(outputDir, to)
        );
    }

    const buildType = ctx.args.indexOf("--debug") >= 0 ? "Debug" : "Release";
    const buildTask = buildWasm(ctx, buildType);
    const assetsTask = buildAssetPackAsync(ctx, path.join(outputDir, "assets"));

    const webManifest = JSON.parse(readTextFileSync(path.join(ctx.sdk.templates, "web/manifest.json")));
    webManifest.name = ctx.title ?? ctx.name;
    webManifest.short_name = ctx.title ?? ctx.name;
    webManifest.description = ctx.desc;
    webManifest.version = ctx.version.shortName();
    webManifest.version_code = ctx.version.buildNumber();
    webManifest.start_url = "./index.html";
    if (ctx.web.applications != null) {
        webManifest.related_applications = ctx.web.applications;
    }

    writeTextFileSync(path.join(outputDir, "manifest.json"), JSON.stringify(webManifest));
    const iconsTask = buildAppIconAsync({
        output: outputDir,
        webManifestIcons: webManifest.icons,
        projectType: "web",
        iconPath: ctx.appIcon
    });

    try {
        await assetsTask;
    } catch (e) {
        logger.error("assets export failed", e);
        throw e;
    }

    logger.info("Do project variables setup..");
    {
        for (const fn of ctx.onProjectGenerated) {
            fn();
        }
    }

    ctx.web.bodyScript.push(render(readTextFileSync(path.join(ctx.sdk.templates, "web/initModule.js"))));

    const pwa = false;
    if (pwa) {
        ctx.web.headCode.push(`<script async src="pwacompat.min.js"></script>`);
        file("web/pwacompat.min.js", "pwacompat.min.js");
        tpl("web/sw.js", "sw.js");

        ctx.web.bodyScript.push(readTextFileSync(path.join(ctx.sdk.templates, "web/initPWA.js")));

        const assetFiles: string[] = [];
        for (const entry of expandGlobSync(path.join(outputDir, "assets/**/*"))) {
            assetFiles.push(`"${path.relative(outputDir, entry.path)}"`);
        }
        const assetsList = assetFiles.join(",\n");
        replaceInFile(path.join(outputDir, "sw.js"), {
            "var contentToCache = [];": `var contentToCache = [
        'index.html',
        'pwacompat.min.js',
        '${ctx.name}.js',
        '${ctx.name}.wasm',
        ${assetsList}
];`
        });
    }

    tpl("web/index.html.mustache", "index.html");

    let buildResult!: BuildResult;
    try {
        buildResult = await buildTask;
    } catch (e) {
        logger.error("build failed", e);
        throw e;
    }
    fs.copyFileSync(path.join(buildResult.buildDir, ctx.name + ".js"), path.join(outputDir, ctx.name + ".js"));
    fs.copyFileSync(path.join(buildResult.buildDir, ctx.name + ".wasm"), path.join(outputDir, ctx.name + ".wasm"));
    try {
        fs.copyFileSync(path.join(buildResult.buildDir, ctx.name + ".wasm.map"), path.join(outputDir, ctx.name + ".wasm.map"));
    } catch {
        // ignore
    }

    if (ctx.args.indexOf("--debug") < 0) {
        await esbuild.build({
            entryPoints: [path.join(outputDir, ctx.name + ".js")],
            target: ["chrome58", "firefox57", "safari11", "edge16"],
            platform: "browser",
            format: "esm",
            minify: true,
            bundle: true,
            outfile: path.join(outputDir, ctx.name + ".js"),
            allowOverwrite: true
        });
    }

    const js_scripts = collectSourceRootsAll(ctx, "js_script", ["web"], ".");
    for (const js_script of js_scripts) {
        fs.copyFileSync(js_script, path.join(outputDir, path.basename(js_script)));
        replaceInFile(path.join(outputDir, "index.html"), {
            "</head>": `    <script src="${path.basename(js_script)}"></script>
</head>`
        });
    }

    try {
        await iconsTask;
    } catch (e) {
        logger.error("icons export failed", e);
        throw e;
    }

    logger.info("Web export completed");
    logger.info("Time:", (Date.now() - timestamp) / 1000, "sec");

    if (ctx.options.deploy != null) {
        await deployFirebaseHosting(ctx);
    }

    if (ctx.options.run != null) {
        await serve(outputDir);
    }
}