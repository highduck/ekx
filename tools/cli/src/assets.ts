import {execute, isDir, makeDirs, optimizePngGlob, optimizePngGlobAsync} from "./utils";
import * as path from "path";
import {Project} from "./project";
import {rmdirSync} from "fs";
import {ekc, ekcAsync} from "./ekc";

export function buildMarketingAssets(ctx: Project, target_type: string, output: string) {
    if (isDir(output)) {
        rmdirSync(output, {recursive: true});
    }
    makeDirs(output);
    const marketAsset = ctx.market_asset ? ctx.market_asset : "assets/res";
    ekc(ctx, "export", "market", marketAsset, target_type, output);

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
    ekc(ctx, "export", "assets", assetsInput, assetsOutput);
    optimizePngGlob(path.join(assetsOutput, "**/*.png"));
}

export async function buildAssetsAsync(ctx: Project, output?: string) {
    let assetsInput = ctx.getAssetsInput();
    let assetsOutput = output ?? ctx.getAssetsOutput();
    if (isDir(assetsOutput)) {
        rmdirSync(assetsOutput, {recursive: true});
    }
    makeDirs(assetsOutput);
    await ekcAsync(ctx, "export", "assets", assetsInput, assetsOutput);
    await optimizePngGlobAsync(path.join(assetsOutput, "**/*.png"));
}