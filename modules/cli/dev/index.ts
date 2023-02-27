import * as path from "path";
import * as fs from "fs";
import {Project} from "../project.js";
import {buildAssetPackAsync} from "../assets.js";
import {resolveEkxPath} from "../../utils/dirs.js";
import {ensureDirSync} from "../../utils/utils.js";

const exportDir = "build";

export async function buildDev(ctx: Project): Promise<any> {
    ctx.generateNativeBuildInfo();

    await buildAssetPackAsync(ctx, path.join(exportDir, "assets"), true);

    ensureDirSync(path.join(exportDir, "dev"));

    const materialIconsFont = resolveEkxPath("external/imgui/ttf/MaterialIcons-Regular.ttf");
    const awesomeIconsFont = resolveEkxPath("external/imgui/ttf/fa-solid-900.ttf");
    const sfPro = resolveEkxPath("external/imgui/ttf/sf-pro-text-regular.ttf");
    const sfMono = resolveEkxPath("external/imgui/ttf/sf-mono-text-regular.ttf");
    fs.copyFileSync(materialIconsFont, path.join(exportDir, "dev/MaterialIcons-Regular.ttf"));
    fs.copyFileSync(awesomeIconsFont, path.join(exportDir, "dev/fa-solid-900.ttf"));
    fs.copyFileSync(sfPro, path.join(exportDir, "dev/sf-pro-text-regular.ttf"));
    fs.copyFileSync(sfMono, path.join(exportDir, "dev/sf-mono-text-regular.ttf"));
}