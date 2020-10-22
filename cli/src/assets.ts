import {execute, is_dir, make_dirs, optimize_png_glob} from "./utils";
import * as path from "path";
import {Project} from "./project";

export function ekc_export_market(ctx:Project, target_type:string, output:string) {
    make_dirs(output);
    execute(path.join(ctx.path.EKX_ROOT, "editor/bin/ekc"), ["export", "market", ctx.market_asset, target_type, output]);
}

export function ekc_export_assets(ctx:Project) {
    let assets_input = "assets";
    let assets_output = ctx.assets.output;
    make_dirs(assets_output);
    execute(path.join(ctx.path.EKX_ROOT, "editor/bin/ekc"), ["export", "assets", assets_input, assets_output]);
    optimize_png_glob(path.join(assets_output, "*.png"));
}

export function ekc_export_assets_lazy(ctx:Project) {
    let assets_output = ctx.assets.output;
    if (!is_dir(assets_output)) {
        ekc_export_assets(ctx);
    }
}
