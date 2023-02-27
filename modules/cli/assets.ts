import {Project} from "./project.js";
import {AssetBuilder} from "./assets/AssetBuilder.js";
import {rm} from "../utils/utils.js";

// asset pack name
export async function buildAssetPackAsync(ctx: Project, output?: string, devMode?: boolean, cleanCache?: boolean) {
    devMode = devMode ?? false;
    let assetsInput = ctx.getAssetsInput();
    let assetsOutput = output ?? ctx.getAssetsOutput();

    const builder = new AssetBuilder(ctx, assetsInput);
    builder.devMode = devMode;
    builder.cache = "export/content/.cache";
    builder.output = assetsOutput;
    if (cleanCache) {
        await rm(builder.cache);
        await rm(builder.output);
    }
    await builder.populate();
    builder.prepare();

    await builder.buildIfChanged();

    if (!devMode) {
        //await optimizePngGlobAsync(path.join(assetsOutput, "**/*.png"));
    }
}