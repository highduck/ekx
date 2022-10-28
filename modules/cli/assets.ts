import {Project} from "./project.js";
import {AssetBuilder} from "./assets/AssetBuilder.js";

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