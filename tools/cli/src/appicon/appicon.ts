import * as path from "path";
import * as fs from "fs";
import {renderFlashSymbol, RenderFlashSymbolOutputOptions} from "../assets/helpers/flashExport";
import {logger} from "../logger";
import {isDir, makeDirs} from "../utils";

export interface WebManifestIcon {
    src: string; // icons/icon36.png
    sizes: string; // 36x36
    type: string; // image/png
}

export interface GenAppIconConfig {
    /**
     * dd/dd.fla#icon
     */
    iconPath?: string;

    projectType: "web" | "android" | "ios";

    output: string;

    webManifestIcons?: WebManifestIcon[];
    iosAppIconContents?: any;
}

export async function buildAppIconAsync(config: GenAppIconConfig) {
    config.iconPath = config.iconPath ?? path.resolve(__dirname, "../../templates/default-icon#icon");
    const parts = config.iconPath.split("#");
    if (parts.length < 2) {
        logger.warn("appicon: missing symbol ref");
        return;
    }

    const flashPath = parts[0];
    const iconSymbol = parts[1];

    switch (config.projectType) {
        case "web":
            await exportWebIcons(flashPath, iconSymbol, config.webManifestIcons, config.output);
            break;
        case "ios":
            await exportIOSIcons(flashPath, iconSymbol, config.iosAppIconContents, config.output);
            break;
        case "android":
            await exportAndroidIcons(flashPath, iconSymbol, config.output);
            break;
    }
}

function exportWebIcons(flashPath: string, iconSymbol: string, webManifestIcons: WebManifestIcon[], output: string) {
    const originalSize = 64.0;
    const outputs: RenderFlashSymbolOutputOptions[] = [];
    for (const iconConfig of webManifestIcons) {
        const size = parseFloat(iconConfig.sizes.split("x")[0]);
        const scale = size / originalSize;
        const outputPath = path.join(output, iconConfig.src);
        outputs.push({
            scale,
            width: size,
            height: size,
            alpha: false,
            trim: true,
            outFilePath: outputPath
        });

        // unsure all dirs will be available before running native rasterizer utility
        makeDirs(path.dirname(outputPath));
    }

    return renderFlashSymbol(flashPath, iconSymbol, outputs);
}

function exportAndroidIcons(flashPath: string, iconSymbol: string, output: string) {
    const filename = "ic_launcher.png";
    const resolutionMap = {
        "ldpi": 36,
        "mdpi": 48,
        "hdpi": 72,
        "xhdpi": 96,
        "xxhdpi": 144,
        "xxxhdpi": 192,
    };

    const originalSize = 64.0;
    const outputs: RenderFlashSymbolOutputOptions[] = [];
    for (const resolution of Object.keys(resolutionMap)) {
        const size = resolutionMap[resolution];
        const outputPath = path.join(output, `mipmap-${resolution}`, filename);
        outputs.push({
            scale: size / originalSize,
            width: size,
            height: size,
            alpha: false,
            trim: true,
            outFilePath: path.join(output, `mipmap-${resolution}`, filename)
        });

        // unsure all dirs will be available before running native rasterizer utility
        makeDirs(path.dirname(outputPath));
    }
    return renderFlashSymbol(flashPath, iconSymbol, outputs);
}

function exportIOSIcons(flashPath: string, iconSymbol: string, appIconContents: any, output: string): Promise<number> {
    const appIconFolder = path.join(output, "AppIcon.appiconset");

    if (isDir(output)) {
        fs.rmdirSync(output, {recursive: true});
    }
    makeDirs(appIconFolder);

    const images = [];
    const originalSize = 64.0;

    const outputs: RenderFlashSymbolOutputOptions[] = [];
    for (const image of appIconContents.images) {
        const scaleFactor = parseFloat(image.scale);
        const sideSize = image.size.substr(0, image.size.indexOf("x"));
        const size = scaleFactor * parseFloat(sideSize);
        const iSize = size | 0;
        const filename = `${image.idiom}_${iSize}.png`;
        image.filename = filename;
        images.push(image);

        outputs.push({
            scale: size / originalSize,
            width: iSize,
            height: iSize,
            alpha: false,
            trim: true,
            outFilePath: path.join(appIconFolder, filename)
        });
    }
    appIconContents.images = images;
    fs.writeFileSync(path.join(appIconFolder, "Contents.json"), JSON.stringify(appIconContents), "utf8");

    return renderFlashSymbol(flashPath, iconSymbol, outputs);
}