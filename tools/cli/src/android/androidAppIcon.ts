import {Project} from "../project";
import * as path from "path";
import {renderFlashSymbol, RenderFlashSymbolOutputOptions} from "../assets/flashExport";

export function androidBuildAppIconAsync(ctx: Project, output: string): Promise<number> {
    const marketAsset = ctx.market_asset ? ctx.market_asset : "assets/res";
    
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
        outputs.push({
            scale: size / originalSize,
            width: size,
            height: size,
            alpha: false,
            trim: true,
            outFilePath: path.join(output, `mipmap-${resolution}`, filename)
        });
    }
    return renderFlashSymbol(marketAsset, "icon", outputs);
}