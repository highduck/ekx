import {Project} from "../project";
import * as path from "path";
import * as fs from "fs";
import {buildAssetPackAsync} from "../assets";
import {resolveFrom} from "../utility/resolveFrom";
import {makeDirs} from "../utils";

const exportDir = "build";

export async function buildDev(ctx: Project): Promise<any> {
    await buildAssetPackAsync(ctx, path.join(exportDir, "assets"), true);

    makeDirs(path.join(exportDir, "dev"));
    // dev-fonts as @ekx/ekx dependency
    const materialIconsFont = resolveFrom(ctx.path.EKX_ROOT, "@ekx/dev-fonts/ttf/MaterialIcons-Regular.ttf");
    const awesomeIconsFont = resolveFrom(ctx.path.EKX_ROOT, "@ekx/dev-fonts/ttf/fa-solid-900.ttf");
    fs.copyFileSync(materialIconsFont, path.join(exportDir, "dev/MaterialIcons-Regular.ttf"));
    fs.copyFileSync(awesomeIconsFont, path.join(exportDir, "dev/fa-solid-900.ttf"));
    fs.copyFileSync(path.join(ctx.path.EKX_ROOT, "dev-tools/assets/sf-pro-text-regular.ttf"), path.join(exportDir, "dev/sf-pro-text-regular.ttf"));
    fs.copyFileSync(path.join(ctx.path.EKX_ROOT, "dev-tools/assets/sf-mono-text-regular.ttf"), path.join(exportDir, "dev/sf-mono-text-regular.ttf"));
}