import {getModuleDir} from "./utils/utils.ts";
import {path} from "./deps.ts";

const urlMap = {
    "linux": "linux/sokol-shdc",
    "darwin": "osx/sokol-shdc",
    "windows": "win32/sokol-shdc.exe"
};

export interface ShdcOptions {
    input: string;
    output: string;
    cwd: string;
}

export async function shdc(options: ShdcOptions) {
    const __dirname = getModuleDir(import.meta);
    const exe = path.resolve(__dirname, "../external/sokol/bin/" + urlMap[Deno.build.os]);
    const cmd = [exe, "-i", options.input, "-o", options.output,
        "-l", "glsl330:glsl300es:glsl100:hlsl5:metal_ios:metal_sim:metal_macos",
        "--ifdef"];
    const process = Deno.run({
        cmd,
        cwd: options.cwd,
        stdout: "inherit",
        stderr: "inherit"
    });
    const status = await process.status();
    return status.success;
}