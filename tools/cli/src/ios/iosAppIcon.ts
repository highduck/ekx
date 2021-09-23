import {isDir, makeDirs, readText, writeText} from "../utils";
import {rmdirSync} from "fs";
import * as path from "path";
import {Project} from "../project";
import {renderFlashSymbol, RenderFlashSymbolOutputOptions} from "../assets/flashExport";

export async function iosBuildAppIconAsync(ctx: Project, output: string): Promise<number> {
    const marketAsset = ctx.market_asset ? ctx.market_asset : "assets/res";
    let iosIcon = JSON.parse(readText(path.join(ctx.path.templates, "template-ios/src/Assets.xcassets/AppIcon.appiconset/Contents.json")));
    const appIconFolder = path.join(output, "AppIcon.appiconset");

    if (isDir(output)) {
        rmdirSync(output, {recursive: true});
    }
    makeDirs(appIconFolder);

    const images = [];
    const originalSize = 64.0;

    const outputs:RenderFlashSymbolOutputOptions[] = [];
    for (const image of iosIcon.images) {
        const scaleFactor = parseFloat(image.scale);
        const sideSize = image.size.substr(0, image.size.indexOf("x"));
        const size = scaleFactor * parseFloat(sideSize);
        const sizei = size | 0;
        const filename = `${image.idiom}_${sizei}.png`;
        image.filename = filename;
        images.push(image);

        outputs.push({
            scale: size / originalSize,
            width: sizei,
            height: sizei,
            alpha: false,
            trim: true,
            outFilePath: path.join(appIconFolder, filename)
        });
    }
    iosIcon.images = images;
    writeText(path.join(appIconFolder, "Contents.json"), JSON.stringify(iosIcon));

    return renderFlashSymbol(marketAsset, "icon", outputs);
}