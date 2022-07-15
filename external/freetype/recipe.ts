import {path, fs} from "../../modules/deps.ts"
import {downloadCheck, untar, copyFolderRecursive, getModuleDir, rm} from "../../modules/utils/mod.ts"

const __dirname = getModuleDir(import.meta);
const cacheDir = path.join(__dirname, "cache");
const tempDir = path.join(__dirname, "tmp");

async function clean() {
    await rm(path.join(__dirname, "include"));
    await rm(path.join(__dirname, "src"));
}

async function removeFilesGlob(glob: string) {
    for await (const file of fs.expandGlob(glob, {root: __dirname})) {
        await Deno.remove(file.path, {recursive: true});
    }
}

async function downloadFreetype() {
    const artifact = "freetype-2.11.0.tar.gz";
    const url = `https://download.savannah.gnu.org/releases/freetype/${artifact}`;
    const sha1 = "e9272ae073e35bb65aa39d55e49a309313f007a7";
    console.log("download freetype");
    await downloadCheck(url, cacheDir, sha1);
    console.log("unpack freetype");
    await untar(path.join(cacheDir, path.basename(url)), tempDir, {strip: 1});
}

async function fetch() {
    await downloadFreetype();
    await copyFolderRecursive(path.join(tempDir, "include"), path.join(__dirname, "include"));
    await copyFolderRecursive(path.join(tempDir, "src"), path.join(__dirname, "src"));
    await Deno.copyFile(path.join(__dirname, "recipe/ftoption.h"), path.join(__dirname, "include/freetype/config/ftoption.h"));
    await Deno.copyFile(path.join(__dirname, "recipe/ft2build.h"), path.join(__dirname, "include/ft2build.h"));
    await removeFilesGlob("src/tools");
    await removeFilesGlob("src/**/*.mk");
    await removeFilesGlob("src/**/README");
    await rm(tempDir);
}

export default {clean, fetch};
