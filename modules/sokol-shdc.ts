import {resolveToolsBinPath} from "./cli/utility/bin.ts";

export interface ShdcOptions {
    input: string;
    output: string;
    cwd?: string | undefined;
}

export async function shdc(options: ShdcOptions) {
    const exe = resolveToolsBinPath("sokol-shdc");
    const cmd = [exe, "-i", options.input, "-o", options.output,
        "-l", "glsl330:glsl300es:glsl100:hlsl5:metal_ios:metal_sim:metal_macos",
        "--ifdef"];
    const process = Deno.run({
        cmd,
        cwd: options.cwd ?? Deno.cwd(),
        stdout: "inherit",
        stderr: "inherit"
    });
    const status = await process.status();
    return status.success;
}