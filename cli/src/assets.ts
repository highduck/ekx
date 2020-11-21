import {execute, is_dir, make_dirs, optimize_png_glob} from "./utils";
import * as path from "path";
import {Project} from "./project";
import {rmdirSync} from "fs";

export function ekc_export_market(ctx: Project, target_type: string, output: string) {
    if (is_dir(output)) {
        rmdirSync(output, {recursive: true});
    }
    make_dirs(output);
    const marketAsset = ctx.market_asset ? ctx.market_asset : "assets/res";
    execute(path.join(ctx.path.EKX_ROOT, "editor/bin/ekc"), ["export", "market", marketAsset, target_type, output]);
    optimize_png_glob(path.join(output, "**/*.png"));
}

export function ekc_export_assets(ctx: Project) {
    let assetsInput = ctx.getAssetsInput();
    let assetsOutput = ctx.getAssetsOutput();
    if (is_dir(assetsOutput)) {
        rmdirSync(assetsOutput, {recursive: true});
    }
    make_dirs(assetsOutput);
    execute(path.join(ctx.path.EKX_ROOT, "editor/bin/ekc"), ["export", "assets", assetsInput, assetsOutput]);
    optimize_png_glob(path.join(assetsOutput, "**/*.png"));
}