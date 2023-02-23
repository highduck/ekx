import * as path from "path";
import * as os from "os";
import {existsSync} from "fs";
import {getModuleDir} from "../../utils/utils.js";
import {build} from "../../cmake/mod.js";
import {logger} from "../logger.js";

export function getCachedBinPath(bin: string): string {
    if (os.platform() === "win32") {
        bin += ".exe";
    }
    return path.resolve(getModuleDir(import.meta), "../../../cache/bin/" + bin);
}

export const tryResolveCachedBin = async (bin: string, resolveBinary: (bin: string, filepath: string) => Promise<any>): Promise<string> => {
    let filepath = getCachedBinPath(bin);
    if (!existsSync(filepath)) {
        await resolveBinary(bin, filepath);
    }
    return filepath;
}

export const getOrBuildUtility = async (bin: string): Promise<string> => {
    return await tryResolveCachedBin(bin, async () => {
        logger.info("Miss " + bin + ", building...");
        await build({
            definitions: {
                EKX_BUILD_DEV_TOOLS: "ON",
                EKX_BUILD_TESTS: "OFF",
                EKX_BUILD_COVERAGE: "OFF",
                EKX_BUILD_EXTERNAL_TESTS: "OFF",
                EKX_INCLUDE_EXAMPLES: "OFF",
                EKX_INCLUDE_PLUGINS: "OFF",
            },
            test: false,
            debug: false,
            target: bin,
            workingDir: path.resolve(getModuleDir(import.meta), "../../.."),
        });
        logger.info(bin + " built");
    });
};
