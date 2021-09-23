import {Project} from "../project";
import * as path from "path";
import {WebManifestIcon} from "./webMeta";
import {renderFlashSymbol, RenderFlashSymbolOutputOptions} from "../assets/flashExport";

export async function webBuildAppIconAsync(ctx: Project, list: WebManifestIcon[], output: string) {
    const marketAsset = ctx.market_asset ? ctx.market_asset : "assets/res";
    const originalSize = 64.0;
    const outputs: RenderFlashSymbolOutputOptions[] = [];
    for (const iconConfig of list) {
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
    }
    return renderFlashSymbol(marketAsset, "icon", outputs);
}