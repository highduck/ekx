import * as path from "path";
import * as fs from "fs";
import {Project} from "../project.js";
import {buildAssetPackAsync} from "../assets.js";
import {makeDirs} from "../utils.js";
import {getModuleDir} from "../../utils/utils.js";

const exportDir = "build";
const __dirname = getModuleDir(import.meta);

export async function buildDev(ctx: Project): Promise<any> {
    ctx.generateNativeBuildInfo();
    
    await buildAssetPackAsync(ctx, path.join(exportDir, "assets"), true);

    makeDirs(path.join(exportDir, "dev"));

    // dev-fonts as @ekx/ekx dependency
    const materialIconsFont = path.resolve(__dirname, "../../../external/imgui/ttf/MaterialIcons-Regular.ttf");
    const awesomeIconsFont = path.resolve(__dirname, "../../../external/imgui/ttf/fa-solid-900.ttf");
    const sfPro = path.resolve(__dirname, "../../../external/imgui/ttf/sf-pro-text-regular.ttf");
    const sfMono = path.resolve(__dirname, "../../../external/imgui/ttf/sf-mono-text-regular.ttf");
    fs.copyFileSync(materialIconsFont, path.join(exportDir, "dev/MaterialIcons-Regular.ttf"));
    fs.copyFileSync(awesomeIconsFont, path.join(exportDir, "dev/fa-solid-900.ttf"));
    fs.copyFileSync(sfPro, path.join(exportDir, "dev/sf-pro-text-regular.ttf"));
    fs.copyFileSync(sfMono, path.join(exportDir, "dev/sf-mono-text-regular.ttf"));
}