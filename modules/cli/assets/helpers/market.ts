import {renderFlashSymbol, RenderFlashSymbolOutputOptions} from "./flashExport.ts";
import {path} from "../../../deps.ts";
import {isDir} from "../../utils.ts";
import {Project} from "../../project.ts";

function getIconOutputs(outPrefix: string, sizes: number[]) {
    const originalSize = 64.0;
    const outputs: RenderFlashSymbolOutputOptions[] = [];
    for (const size of sizes) {
        outputs.push({
            scale: size / originalSize,
            width: size,
            height: size,
            alpha: true,
            trim: true,
            outFilePath: outPrefix + size + ".png"
        });
    }
    return outputs;
}

async function market(fla: string = "assets/res", out: string) {
    await renderFlashSymbol(fla, "icon_market", getIconOutputs(path.join(out, "icon_"), [16, 32, 256, 512, 1024]));
    await renderFlashSymbol(fla, "feature_graphic", [{
        scale: 1.0,
        width: 1024,
        height: 512,
        alpha: false,
        trim: true,
        outFilePath: path.join(out, "_feature_graphic.png")
    }]);
    await renderFlashSymbol(fla, "promo_graphic", [{
        scale: 1.0,
        width: 180,
        height: 120,
        alpha: false,
        trim: true,
        outFilePath: path.join(out, "_promo_graphic.png")
    }]);
    await renderFlashSymbol(fla, "btn_app_store", [{
        scale: 2.0,
        width: 0,
        height: 0,
        alpha: false,
        trim: false,
        outFilePath: path.join(out, "btn_app_store.png")
    }]);
    await renderFlashSymbol(fla, "btn_google_play", [{
        scale: 2.0,
        width: 0,
        height: 0,
        alpha: false,
        trim: false,
        outFilePath: path.join(out, "btn_google_play.png")
    }]);
    // for (const auto& s: exporter.doc.library) {
    //     if (starts_with(s.item.name, "achievements/") || starts_with(s.item.name, "leaderboards/")) {
    //         auto dir = path_t{s.item.name}.dir();
    //         make_dirs(dir);
    //         wd.in(dir, [&]() {
    //             RenderElementOptions opts{1.0f, 512, 512, false, false};
    //             auto spr = renderElement(doc, s, opts);
    //             save_sprite_png(spr, path_t{s.timeline.name + ".png"});
    //             destroy_sprite_data(spr);
    //         });
    //     }
}

export async function buildMarketingAssets(ctx: Project, output: string):Promise<number> {
    if (isDir(output)) {
        Deno.removeSync(output, {recursive: true});
    }
    const marketAsset = ctx.market_asset ? ctx.market_asset : "assets/res";
    await market(marketAsset, output);
    return 0;
}
