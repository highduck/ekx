import {isDir, makeDirs, optimizePngGlobAsync} from "./utils";
import * as path from "path";
import {Project} from "./project";
import {rmdirSync} from "fs";
import {ekcAsync} from "./assets/ekc";
import {AssetBuilder} from "./assets/ap";
import {
    AudioAsset,
    BitmapFontAsset,
    DynamicAtlasAsset,
    FlashAsset,
    ModelAsset,
    MultiResAtlasAsset,
    TextureAsset,
    TranslationsAsset,
    TTFAsset
} from "./assets/apr";

// asset pack name
export async function buildAssetPackAsync(ctx: Project, output?: string, devMode?: boolean) {
    devMode = devMode ?? false;
    let assetsInput = ctx.getAssetsInput();
    let assetsOutput = output ?? ctx.getAssetsOutput();
    if (isDir(assetsOutput)) {
        rmdirSync(assetsOutput, {recursive: true});
    }

    const builder = new AssetBuilder(assetsInput);
    builder.factory.set("ttf", p => new TTFAsset(p));
    builder.factory.set("audio", p => new AudioAsset(p));
    builder.factory.set("model", p => new ModelAsset(p));
    builder.factory.set("texture", p => new TextureAsset(p));
    builder.factory.set("flash", p => new FlashAsset(p));
    builder.factory.set("bmfont", p => new BitmapFontAsset(p));
    builder.factory.set("atlas_builder", p => new MultiResAtlasAsset(p));
    builder.factory.set("dynamic_atlas", p => new DynamicAtlasAsset(p));
    builder.factory.set("translations", p => new TranslationsAsset(p));
    builder.devMode = devMode;
    builder.populate();
    await builder.build(assetsOutput, "export/content/.cache");

    if (!devMode) {
        //await optimizePngGlobAsync(path.join(assetsOutput, "**/*.png"));
    }
}