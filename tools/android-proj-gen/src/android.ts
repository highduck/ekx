import * as path from "path";
import {execSync, spawnSync} from "child_process";

export function getAndroidSdkRoot(): string | null {
    return process.env.ANDROID_SDK_ROOT ?? path.join(process.env.HOME, 'Library/Android/sdk');
}

export function getJavaHome(version: string | number): string | null {
    try {
        return execSync(`/usr/libexec/java_home -v ${version}`, {
            encoding: 'utf-8'
        });
    } catch {
    }
    console.warn("java_home not found");
    return null;
}

function getAndroidStudioPath(): null | string {
    switch (process.platform) {
        case "darwin":
            return "/Applications/Android Studio.app";
    }
    console.error("Not supported");
    return null;
}

export function openAndroidStudioProject(projectPath: string): void {
    switch (process.platform) {
        case "darwin":
            spawnSync("open", ["-a", getAndroidStudioPath(), projectPath]);
            break;
        default:
            console.error("Not supported");
            break;
    }
}