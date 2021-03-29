import {Project} from "../project";
import * as path from "path";
import {isDir, makeDirs} from "../utils";
import {rmdirSync} from "fs";
import {ekcAsync} from "../ekc";
import {WebManifestIcon} from "./webMeta";

export async function webBuildAppIconAsync(ctx: Project, list:WebManifestIcon[], output: string) {
    const marketAsset = ctx.market_asset ? ctx.market_asset : "assets/res";
    if (isDir(output)) {
        rmdirSync(output, {recursive: true});
    }
    makeDirs(output);

    const originalSize = 64.0;
    let cmd = ["prerender_flash", marketAsset, "icon"];
    for (const iconConfig of list) {
        const size = parseFloat(iconConfig.sizes.split("x")[0]);
        const scale = size / originalSize;
        cmd.push(scale.toString(), size.toString(), size.toString(), "0", "1", path.join(output, iconConfig.src));
    }
    return ekcAsync(ctx, ...cmd);
}