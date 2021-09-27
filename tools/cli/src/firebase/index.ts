import {Project} from "../project";
import * as path from "path";
import {executeAsync} from "../utils";
import {existsSync, renameSync, rmSync} from "fs";

async function updateGS(ctx: Project, configDir: string, name: string, platform: string) {
    if (!configDir) {
        return;
    }
    const baseDir = path.resolve(ctx.projectPath, configDir);
    const fileNew = path.join(baseDir, "new_" + name);
    const file = path.join(baseDir, name);

    await executeAsync("firebase", ["apps:sdkconfig", "-o", fileNew, platform]);
    if (existsSync(fileNew)) {
        rmSync(file, {force: true});
        renameSync(fileNew, file)
    }
}

export async function updateGoogleServicesConfig(ctx: Project): Promise<any> {
    return Promise.all([
        updateGS(ctx, ctx.android.googleServicesConfigDir, "google-services.json", "android"),
        updateGS(ctx, ctx.ios.googleServicesConfigDir, "GoogleService-Info.plist", "ios")
    ]);
}