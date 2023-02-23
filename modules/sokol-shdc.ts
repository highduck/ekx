import {tryResolveCachedBin} from "./cli/utility/bin.js";
import {ensureDirSync, run} from "./utils/utils.js";
import * as path from "path";
import * as os from "os";
import * as fs from "fs";
import {download} from "./utils/download.js";
import {logger} from "./cli/logger.js";

export interface ShdcOptions {
    input: string;
    output: string;
    cwd?: string | undefined;
}

export const getOrFetchSHDC = async (): Promise<string> => {
    return await tryResolveCachedBin("sokol-shdc", async (bin, exePath) => {
        logger.info("Miss " + bin);
        logger.info("Download sokol-shdc...");
        const platform = os.platform();
        // download shdc tool
        const exeURL = ({
            linux: "bin/linux/sokol-shdc",
            darwin: "bin/osx/sokol-shdc",
            win32: "bin/win32/sokol-shdc.exe",
        } as any)[platform];
        ensureDirSync(path.dirname(exePath));
        await download("https://github.com/floooh/sokol-tools-bin/raw/master/" + exeURL, exePath);
        if (platform !== "win32") {
            fs.chmodSync(exePath, 0o755);
        }
    });
};

export async function shdc(options: ShdcOptions) {
    const exePath = await getOrFetchSHDC();
    const res = await run({
        cmd: [
            exePath,
            "-i", options.input, "-o", options.output,
            "-l", "glsl330:glsl300es:glsl100:hlsl5:metal_ios:metal_sim:metal_macos",
            "--ifdef"
        ],
        cwd: options.cwd,
    });
    return res.success;
}