import {execute, isDir, makeDirs, optimizePngGlob, optimizePngGlobAsync} from "./utils";
import * as path from "path";
import {Project} from "./project";
import {rmdirSync} from "fs";
import {ekc, ekcAsync} from "./ekc";

export function buildMarketingAssets(ctx: Project, target_type: string, output: string):Promise<number> {
    if (isDir(output)) {
        rmdirSync(output, {recursive: true});
    }
    makeDirs(output);
    const marketAsset = ctx.market_asset ? ctx.market_asset : "assets/res";
    return ekcAsync(ctx, "export", "market", marketAsset, target_type, output);
}

export async function buildAssetsAsync(ctx: Project, output?: string, devMode?: boolean) {
    devMode = devMode ?? false;
    let assetsInput = ctx.getAssetsInput();
    let assetsOutput = output ?? ctx.getAssetsOutput();
    if (isDir(assetsOutput)) {
        rmdirSync(assetsOutput, {recursive: true});
    }
    makeDirs(assetsOutput);
    const args = ["export", "assets", assetsInput, assetsOutput];
    if (devMode) {
        args.push("--dev");
    }
    await ekcAsync(ctx, ...args);
    if (!devMode) {
        await optimizePngGlobAsync(path.join(assetsOutput, "**/*.png"));
    }
}