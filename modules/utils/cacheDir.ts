import {ensureDirSync, getModuleDir} from "./utils.js";
import * as path from "path";

const __dirname = getModuleDir(import.meta);

export const resolveEkxPath = (...paths: string[]): string => path.resolve(__dirname, "../..", ...paths);

export const resolveCachePath = (...paths: string[]): string => resolveEkxPath("cache", ...paths);

export const ensureCacheDir = (...paths: string[]): string => {
    const cacheDir = resolveCachePath(...paths);
    ensureDirSync(cacheDir);
    return cacheDir;
};
