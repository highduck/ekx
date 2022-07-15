import {path} from "../deps.ts"
import {logger} from "../cli/logger.ts";

export function getAndroidSdkRoot(): string | null {
    return Deno.env.get("ANDROID_SDK_ROOT") ?? path.join(Deno.env.get("HOME") ?? "~", 'Library/Android/sdk');
}

export async function getJavaHome(version: string | number): Promise<string> {
    const process = Deno.run({cmd: ["/usr/libexec/java_home", "-v", version.toString()], stdout: "piped"});
    const output = await process.output();
    process.close();
    return new TextDecoder().decode(output);
}

function getAndroidStudioPath(): string {
    switch (Deno.build.os) {
        case "darwin":
            return "/Applications/Android Studio.app";
    }
    throw new Error("Not supported");
}

export function openAndroidStudioProject(projectPath: string): void {
    switch (Deno.build.os) {
        case "darwin":
            Deno.run({cmd: ["open", "-a", getAndroidStudioPath(), projectPath]}).status().catch((err) => logger.error(err));
            break;
        default:
            console.error("Not supported");
            break;
    }
}