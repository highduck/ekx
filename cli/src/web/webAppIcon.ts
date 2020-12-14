import {Project} from "../project";
import * as path from "path";
import {execute, isDir, makeDirs} from "../utils";
import {rmdirSync} from "fs";

export function webBuildAppIcon(ctx: Project, output: string) {
    const marketAsset = ctx.market_asset ? ctx.market_asset : "assets/res";
    const binEKC = path.join(ctx.path.EKX_ROOT, "editor/bin/ekc");

    if (isDir(output)) {
        rmdirSync(output, {recursive: true});
    }
    makeDirs(output);

    const resolutions = [36, 48, 72, 96, 144, 192, 256, 512];

    const originalSize = 64.0;

    let cmd = ["prerender_flash", marketAsset, "icon"];
    for (const size of resolutions) {
        const scale = size / originalSize;
        cmd.push(scale.toString(), size.toString(), size.toString(), "0", "1", path.join(output, `icon${size}.png`));
    }
    execute(binEKC, cmd);
}