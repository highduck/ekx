import * as path from "path"
import * as os from "os"
import {logger} from "../cli/logger.js";
import {run} from "../utils/utils.js";

export function getAndroidSdkRoot(): string | null {
    return process.env.ANDROID_SDK_ROOT ?? path.join(process.env.HOME ?? "~", 'Library/Android/sdk');
}

export async function getJavaHome(version: string | number): Promise<string> {
    const result = await run({cmd: ["/usr/libexec/java_home", "-v", version.toString()], io: true});
    return result.data;
}

function getAndroidStudioPath(): string {
    switch (os.platform()) {
        case "darwin":
            return "/Applications/Android Studio.app";
    }
    throw new Error("Not supported");
}

export function openAndroidStudioProject(projectPath: string): void {
    switch (os.platform()) {
        case "darwin":
            run({cmd: ["open", "-a", getAndroidStudioPath(), projectPath]}).catch((err) => logger.error(err));
            break;
        default:
            console.error("Not supported");
            break;
    }
}