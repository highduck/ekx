import * as path from "path";
import * as os from "os";
import {existsSync} from "fs";
import {getModuleDir} from "../../utils/utils.js";
import {build} from "../../cmake/mod.js";
import {logger} from "../logger.js";
import {resolveCachePath} from "../../utils/cacheDir.js";

const __dirname = getModuleDir(import.meta);

export function getCachedBinPath(bin: string): string {
    if (os.platform() === "win32") {
        bin += ".exe";
    }
    return resolveCachePath("bin/" + bin);
}

const resolveTasks = new Map<string, Promise<void>>();

export const tryResolveCachedBin = async (bin: string, resolveBinary: (bin: string, filepath: string) => Promise<any>): Promise<string> => {
    let filepath = getCachedBinPath(bin);
    if (!existsSync(filepath)) {
        const resolveTask = resolveTasks.get(bin);
        if (resolveTask) {
            await resolveTask;
        } else {
            const task = resolveBinary(bin, filepath);
            resolveTasks.set(bin, task);
            await task;
            resolveTasks.delete(bin);
        }
    }
    return filepath;
}

let sdkBuildQueue: Promise<unknown> = Promise.resolve();
export const getOrBuildUtility = async (bin: string): Promise<string> => {
    return await tryResolveCachedBin(bin, () =>
        sdkBuildQueue = sdkBuildQueue.then(async () => {
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
                // set main EKX root dir for `CMakeFiles.txt`
                workingDir: path.resolve(__dirname, "../../.."),
                // place build files in central cache folder
                buildsFolder: resolveCachePath("builds/tools"),
            });
            logger.info(bin + " built");
        })
    );
};
