import * as fs from "fs";
import * as path from "path";
import {resolveCachePath, resolveEkxPath} from "../../../modules/utils/cacheDir.js";
import {copyFolderRecursive, expandGlobSync, getModuleDir, rm} from "../../../modules/utils/utils.js";
import {downloadCheck, untar} from "../../../modules/utils/download.js";
import {logger} from "../../../modules/cli/index.js";

const destDir = resolveEkxPath("external/freetype");
const patchDir = path.join(getModuleDir(import.meta), "recipe");
const cacheDir = resolveCachePath("external/freetype/artifacts");
const tempDir = resolveCachePath("external/freetype/sources");

async function clean() {
    await rm(path.join(destDir, "include"));
    await rm(path.join(destDir, "src"));
}

async function removeFilesGlob(glob: string) {
    for (const file of expandGlobSync(glob, {root: destDir})) {
        await rm(file.path);
    }
}

async function downloadFreetype() {
    const artifact = "freetype-2.11.0.tar.gz";
    const url = `https://download.savannah.gnu.org/releases/freetype/${artifact}`;
    const sha1 = "e9272ae073e35bb65aa39d55e49a309313f007a7";
    logger.log("download freetype");
    await downloadCheck(url, cacheDir, sha1);
    logger.log("unpack freetype");
    await untar(path.join(cacheDir, path.basename(url)), tempDir, {strip: 1});
}

async function fetch() {
    await downloadFreetype();
    await copyFolderRecursive(path.join(tempDir, "include"), path.join(destDir, "include"));
    await copyFolderRecursive(path.join(tempDir, "src"), path.join(destDir, "src"));
    await fs.promises.copyFile(path.join(patchDir, "ftoption.h"), path.join(destDir, "include/freetype/config/ftoption.h"));
    await fs.promises.copyFile(path.join(patchDir, "ft2build.h"), path.join(destDir, "include/ft2build.h"));
    await removeFilesGlob("src/tools");
    await removeFilesGlob("src/**/*.mk");
    await removeFilesGlob("src/**/README");
    await rm(tempDir);
}

export default {clean, fetch};
