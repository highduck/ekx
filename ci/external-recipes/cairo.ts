import * as path from "path";
import {
    copyFolderRecursive,
    expandGlobSync,
    readTextFileSync,
    rm,
    writeTextFileSync
} from "../../modules/utils/utils.js";
import {downloadCheck, untar} from "../../modules/utils/download.js";
import {resolveCachePath, resolveEkxPath} from "../../modules/utils/dirs.js";
import {logger} from "../../modules/cli/logger.js";

const destDir = resolveEkxPath("external/cairo");
const cacheDir = resolveCachePath("external/cairo/artifacts");
const tempDir = resolveCachePath("external/cairo/sources");

async function clean() {
    await rm(path.join(destDir, "src"));
}

async function downloadCairo() {
    const url = "https://www.cairographics.org/snapshots/cairo-1.17.4.tar.xz";
    const sha1 = "68712ae1039b114347be3b7200bc1c901d47a636";
    logger.log("download cairo");
    await downloadCheck(url, cacheDir, sha1);
    logger.log("unpack cairo");
    await untar(path.join(cacheDir, path.basename(url)), path.join(tempDir, "cairo"), {strip: 1});
}

async function downloadPixMan() {
    const url = "https://www.cairographics.org/snapshots/pixman-0.33.6.tar.gz";
    const sha1 = "f174d00517e7e1d81c90c65efc20dd876877d904";
    logger.log("download pixman");
    await downloadCheck(url, cacheDir, sha1);
    logger.log("unpack pixman");
    await untar(path.join(cacheDir, path.basename(url)), path.join(tempDir, "pixman"), {strip: 1});
}

async function removeFilesGlob(glob: string) {
    for (const file of expandGlobSync(glob, {root: destDir})) {
        await rm(file.path);
    }
}

async function fetch() {
    await Promise.all([downloadCairo(), downloadPixMan()]);

    logger.log("patch files");
    writeTextFileSync(path.join(tempDir, "cairo/src/cairo-features.h"), `#ifndef CAIRO_FEATURES_H
#define CAIRO_FEATURES_H

#define HAVE_STDINT_H 1
#define HAVE_CXX11_ATOMIC_PRIMITIVES 1
#define CAIRO_HAS_PTHREAD 1
#define HAVE_UINT64_T 1
#define HAVE_UINT128_T 0

// required by cairo_fopen for _WIN32
#define CAIRO_HAS_UTF8_TO_UTF16 1

// ignore float pixel format
#define PIXMAN_rgba_float 0xFFFFFFFF
#define PIXMAN_rgb_float 0xFFFFFFFE

#endif //CAIRO_FEATURES_H
`);

    writeTextFileSync(path.join(tempDir, "pixman/pixman/pixman-config.h"), `#ifndef PIXMAN_CONFIG_H
#define PIXMAN_CONFIG_H

#define PACKAGE
#define HAVE_PTHREADS 1

#endif //PIXMAN_CONFIG_H
`);

    writeTextFileSync(path.join(tempDir, "pixman/pixman/pixman-private.h"),
        '#include <pixman-config.h>\n' + readTextFileSync(path.join(tempDir, "pixman/pixman/pixman-private.h"))
    );

    logger.log("copy to src");
    await copyFolderRecursive(path.join(tempDir, "cairo/src"), path.join(destDir, "src"));
    await copyFolderRecursive(path.join(tempDir, "pixman/pixman"), path.join(destDir, "src"));

    logger.log("cleanup src");
    await removeFilesGlob("src/**/*.in");
    await removeFilesGlob("src/**/*.sh");
    await removeFilesGlob("src/**/*.awk");
    await removeFilesGlob("src/**/*.build");
    await removeFilesGlob("src/**/Makefile*");
    await removeFilesGlob("src/**/README");

    await rm(tempDir);
}

export default {clean, fetch};
