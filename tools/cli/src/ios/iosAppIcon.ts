import {isDir, makeDirs, readText, writeText} from "../utils";
import {rmdirSync} from "fs";
import * as path from "path";
import {Project} from "../project";
import {ekcAsync} from "../ekc";

export async function iosBuildAppIconAsync(ctx: Project, output: string): Promise<number> {
    const marketAsset = ctx.market_asset ? ctx.market_asset : "assets/res";
    let iosIcon = JSON.parse(readText(path.join(ctx.path.templates, "template-ios/src/Assets.xcassets/AppIcon.appiconset/Contents.json")));
    const appIconFolder = path.join(output, "AppIcon.appiconset");

    if (isDir(output)) {
        rmdirSync(output, {recursive: true});
    }
    makeDirs(appIconFolder);

    const images = [];
    const original_size = 64.0;

    let cmd = ["prerender_flash", marketAsset, "icon"];
    for (const image of iosIcon.images) {
        const scale_factor = parseFloat(image.scale);
        const sideSize = image.size.substr(0, image.size.indexOf("x"));
        const size = scale_factor * parseFloat(sideSize);
        const sizei = size | 0;
        let filename = `${image.idiom}_${sizei}.png`;
        const scale = size / original_size;

        cmd.push(scale.toString(), sizei.toString(), sizei.toString(), "0", "1", path.join(appIconFolder, filename));
        image.filename = filename;
        images.push(image);
    }
    iosIcon.images = images;
    writeText(path.join(appIconFolder, "Contents.json"), JSON.stringify(iosIcon));

    return ekcAsync(ctx, ...cmd);
}