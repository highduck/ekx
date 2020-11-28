import * as fs from "fs";
import * as path from "path";
import {copyFolderRecursiveSync} from "./utils";
import {buildAssets, buildMarketingAssets} from "./assets";
import * as Mustache from 'mustache';

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
    buildMarketingAssets(ctx, "web", path.join(output_dir, "icons"));

    tpl("web/index.html.mustache", "index.html");
    tpl("web/manifest.json.mustache", "manifest.webmanifest");
    tpl("web/sw.js.mustache", "sw.js");
    file("web/howler.core.min.js", "howler.core.min.js");
    file("web/pwacompat.min.js", "pwacompat.min.js");

    copyFolderRecursiveSync("export/contents/assets", "export/web/assets");
}