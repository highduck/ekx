import {resolveToolsBinPath} from "./cli/utility/bin.js";
import {run} from "./utils/utils.js";

export interface ShdcOptions {
    input: string;
    output: string;
    cwd?: string | undefined;
}

export async function shdc(options: ShdcOptions) {
    const res = await run({
        cmd: [
            resolveToolsBinPath("sokol-shdc"),
            "-i", options.input, "-o", options.output,
            "-l", "glsl330:glsl300es:glsl100:hlsl5:metal_ios:metal_sim:metal_macos",
            "--ifdef"
        ],
        cwd: options.cwd,
    });
    return res.success;
}