import {Project} from "../project";
import * as path from "path";
import {execute, isDir, makeDirs, readText, writeText} from "../utils";
import {rmdirSync} from "fs";

export function androidBuildAppIcon(ctx: Project, output: string) {
    const marketAsset = ctx.market_asset ? ctx.market_asset : "assets/res";
    const binEKC = path.join(ctx.path.EKX_ROOT, "editor/bin/ekc");

    if (isDir(output)) {
        rmdirSync(output, {recursive: true});
    }
    makeDirs(output);

    const filename = "ic_launcher.png";
    const resolutionMap = {
        "ldpi":    36,
        "mdpi":    48,
        "hdpi":    72,
        "xhdpi":   96,
        "xxhdpi":  144,
        "xxxhdpi": 192,
    };

    const originalSize = 64.0;

    let cmd = ["prerender_flash", marketAsset, "icon"];
    for (const resolution of Object.keys(resolutionMap)) {
        const size = resolutionMap[resolution];
        const scale = size / originalSize;
        const dir = path.join(output, `mipmap-${resolution}`);
        makeDirs(dir);
        cmd.push(scale.toString(), size.toString(), size.toString(), "0", "1", path.join(dir, filename));
    }
    execute(binEKC, cmd);
}