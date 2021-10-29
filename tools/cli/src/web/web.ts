import * as fs from "fs";
import {copyFileSync} from "fs";
import * as path from "path";
import {makeDirs, replaceInFile} from "../utils";
import {buildAssetPackAsync} from "../assets";
import * as Mustache from 'mustache';
import {Project} from "../project";
import {BuildResult} from "cmake-build";
import {serve} from "./serve";
import {logger} from "../logger";
import * as glob from "glob";
import {buildWasm} from "./buildWasm";
import {deployFirebaseHosting} from "./deployFirebaseHosting";
import {buildAppIconAsync} from "../appicon/appicon";

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
    makeDirs(outputDir);

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
            bodyCode: `<script>${ctx.web.bodyCode.join("\n")}</script>`,
            headCode: ctx.web.headCode.join("\n")
        };
        return Mustache.render(str, renderParameters);
    }

    function tpl(from, to) {
        const tplContent = fs.readFileSync(path.join(ctx.sdk.templates, from), "utf8");
        fs.writeFileSync(path.join(outputDir, to), render(tplContent), "utf8");
    }

    function file(from, to) {
        fs.copyFileSync(
            path.join(ctx.sdk.templates, from),
            path.join(outputDir, to)
        );
    }

    const buildType = ctx.args.indexOf("--debug") >= 0 ? "Debug" : "Release";
    const buildTask = buildWasm(ctx, buildType);
    const assetsTask = buildAssetPackAsync(ctx, path.join(outputDir, "assets"));

    const webManifest = JSON.parse(fs.readFileSync(path.join(ctx.sdk.templates, "web/manifest.json"), "utf8"));
    webManifest.name = ctx.title ?? ctx.name;
    webManifest.short_name = ctx.title ?? ctx.name;
    webManifest.description = ctx.desc;
    webManifest.version = ctx.version.shortName();
    webManifest.version_code = ctx.version.buildNumber();
    webManifest.start_url = "./index.html";
    if (ctx.web.applications != null) {
        webManifest.related_applications = ctx.web.applications;
    }

    fs.writeFileSync(path.join(outputDir, "manifest.json"), JSON.stringify(webManifest), "utf8");
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

    ctx.web.bodyCode.push(render(fs.readFileSync(path.join(ctx.sdk.templates, "web/initModule.js"), "utf8")));

    const pwa = false;
    if (pwa) {
        ctx.web.headCode.push(`<script async src="pwacompat.min.js"></script>`);
        file("web/pwacompat.min.js", "pwacompat.min.js");
        tpl("web/sw.js", "sw.js");

        ctx.web.bodyCode.push(fs.readFileSync(path.join(ctx.sdk.templates, "web/initPWA.js"), "utf8"));

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
    }

    tpl("web/index.html.mustache", "index.html");

    let buildResult!: BuildResult;
    try {
        buildResult = await buildTask;
    } catch (e) {
        logger.error("build failed", e);
        throw e;
    }
    copyFileSync(path.join(buildResult.buildDir, ctx.name + ".js"), path.join(outputDir, ctx.name + ".js"));
    copyFileSync(path.join(buildResult.buildDir, ctx.name + ".wasm"), path.join(outputDir, ctx.name + ".wasm"));
    try {
        copyFileSync(path.join(buildResult.buildDir, ctx.name + ".wasm.map"), path.join(outputDir, ctx.name + ".wasm.map"));
    } catch {
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