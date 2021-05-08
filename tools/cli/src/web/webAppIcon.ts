import {Project} from "../project";
import * as path from "path";
import {makeDirs} from "../utils";
import {ekcAsync} from "../ekc";
import {WebManifestIcon} from "./webMeta";

export async function webBuildAppIconAsync(ctx: Project, list:WebManifestIcon[], output: string) {
    const marketAsset = ctx.market_asset ? ctx.market_asset : "assets/res";
    const originalSize = 64.0;
    let cmd = ["prerender_flash", marketAsset, "icon"];
    for (const iconConfig of list) {
        const size = parseFloat(iconConfig.sizes.split("x")[0]);
        const scale = size / originalSize;
        const outputPath = path.join(output, iconConfig.src);
        makeDirs(path.dirname(outputPath));
        cmd.push(scale.toString(), size.toString(), size.toString(), "0", "1", outputPath);
    }
    return ekcAsync(ctx, ...cmd);
}