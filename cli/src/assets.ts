import {execute, isDir, makeDirs, optimizePngGlob} from "./utils";
import * as path from "path";
import {Project} from "./project";
import {rmdirSync} from "fs";

export function buildMarketingAssets(ctx: Project, target_type: string, output: string) {
    if (isDir(output)) {
        rmdirSync(output, {recursive: true});
    }
    makeDirs(output);
    const marketAsset = ctx.market_asset ? ctx.market_asset : "assets/res";
    execute(path.join(ctx.path.EKX_ROOT, "editor/bin/ekc"), ["export", "market", marketAsset, target_type, output]);

    // why optimize market assets? :)
    //optimizePngGlob(path.join(output, "**/*.png"));
}

export function buildAssets(ctx: Project, output?: string) {
    let assetsInput = ctx.getAssetsInput();
    let assetsOutput = output ?? ctx.getAssetsOutput();
    if (isDir(assetsOutput)) {
        rmdirSync(assetsOutput, {recursive: true});
    }
    makeDirs(assetsOutput);
    execute(path.join(ctx.path.EKX_ROOT, "editor/bin/ekc"), ["export", "assets", assetsInput, assetsOutput]);
    optimizePngGlob(path.join(assetsOutput, "**/*.png"));
}