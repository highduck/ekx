import {isDir, makeDirs} from "./utils.ts";
import {Project} from "./project.ts";
import {AssetBuilder} from "./assets/AssetBuilder.ts";
import {logger} from "./logger.ts";

// asset pack name
export async function buildAssetPackAsync(ctx: Project, output?: string, devMode?: boolean) {
    devMode = devMode ?? false;
    let assetsInput = ctx.getAssetsInput();
    let assetsOutput = output ?? ctx.getAssetsOutput();

    const builder = new AssetBuilder(ctx, assetsInput);
    builder.devMode = devMode;
    builder.cache =  "export/content/.cache";
    builder.output = assetsOutput;
    await builder.populate();
    builder.prepare();

    await builder.buildIfChanged();

    if (!devMode) {
        //await optimizePngGlobAsync(path.join(assetsOutput, "**/*.png"));
    }
}