import {os, path} from "../deps.ts"
import {logger} from "../cli/logger.ts";

export function getAndroidSdkRoot(): string | null {
    return Deno.env.get("ANDROID_SDK_ROOT") ?? path.join(Deno.env.get("HOME") ?? "~", 'Library/Android/sdk');
}

export async function getJavaHome(version: string | number): Promise<string> {
    //try {
    const output = await Deno.run({cmd: ["/usr/libexec/java_home", "-v", version.toString()], stdout: "piped"}).output();
    return new TextDecoder().decode(output);
    // } catch {
    //     // ignore
    // }
    // throw new Error("java_home not found");
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
            Deno.run({cmd: ["open", "-a", getAndroidStudioPath(), projectPath]}).status().catch((err)=>logger.error(err));
            break;
        default:
            console.error("Not supported");
            break;
    }
}