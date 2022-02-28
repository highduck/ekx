import {isDir, makeDirs} from "./utils";
import {Project} from "./project";
import {rmdirSync} from "fs";
import {AssetBuilder} from "./assets/AssetBuilder";
import {logger} from "./logger";

// asset pack name
export async function buildAssetPackAsync(ctx: Project, output?: string, devMode?: boolean) {
    devMode = devMode ?? false;
    let assetsInput = ctx.getAssetsInput();
    let assetsOutput = output ?? ctx.getAssetsOutput();

    const builder = new AssetBuilder(ctx, assetsInput);
    builder.devMode = devMode;
    builder.cache =  "export/content/.cache";
    builder.output = assetsOutput;
    builder.populate();
    builder.prepare();

    await builder.buildIfChanged();

    if (!devMode) {
        //await optimizePngGlobAsync(path.join(assetsOutput, "**/*.png"));
    }
}