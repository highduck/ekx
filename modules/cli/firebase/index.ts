import * as path from "path";
import * as fs from "fs";
import {Project} from "../project.js";
import {execute} from "../utils.js";

async function updateGS(ctx: Project, configDir: string, name: string, platform: string) {
    if (!configDir) {
        return;
    }
    const baseDir = path.resolve(ctx.projectPath, configDir);
    const fileNew = path.join(baseDir, "new_" + name);
    const file = path.join(baseDir, name);

    await execute("firebase", ["apps:sdkconfig", "-o", fileNew, platform]);
    if (fs.existsSync(fileNew)) {
        fs.rmSync(file);
        fs.renameSync(fileNew, file)
    }
}

export function updateGoogleServicesConfig(ctx: Project): Promise<any> {
    return Promise.all([
        updateGS(ctx, ctx.android.googleServicesConfigDir!, "google-services.json", "android"),
        updateGS(ctx, ctx.ios.googleServicesConfigDir!, "GoogleService-Info.plist", "ios")
    ]);
}