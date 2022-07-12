import {path, fs} from "ekx/deps.ts"
import {downloadCheck, untar} from "ekx/utils/mod.ts"
import {copyFolderRecursive, getModuleDir, rm} from "ekx/utils/mod.ts"

const __dirname = getModuleDir(import.meta);
const cacheDir = path.join(__dirname, "cache");
const tempDir = path.join(__dirname, "tmp");

async function clean() {
    await rm(path.join(__dirname, "src"));
}

async function downloadCairo() {
    const url = "https://www.cairographics.org/snapshots/cairo-1.17.4.tar.xz";
    const sha1 = "68712ae1039b114347be3b7200bc1c901d47a636";
    console.log("download cairo");
    await downloadCheck(url, cacheDir, sha1);
    console.log("unpack cairo");
    await untar(path.join(cacheDir, path.basename(url)), path.join(tempDir, "cairo"), {strip: 1});
}

async function downloadPixMan() {
    const url = "https://www.cairographics.org/snapshots/pixman-0.33.6.tar.gz";
    const sha1 = "f174d00517e7e1d81c90c65efc20dd876877d904";
    console.log("download pixman");
    await downloadCheck(url, cacheDir, sha1);
    console.log("unpack pixman");
    await untar(path.join(cacheDir, path.basename(url)), path.join(tempDir, "pixman"), {strip: 1});
}

async function removeFilesGlob(glob: string) {
    for await (const file of fs.expandGlob(glob, {root: __dirname})) {
        await Deno.remove(file.path);
    }
}

async function fetch() {
    await Promise.all([downloadCairo(), downloadPixMan()]);

    console.log("patch files");
    await Deno.writeTextFile(path.join(tempDir, "cairo/src/cairo-features.h"), `#ifndef CAIRO_FEATURES_H
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

    await Deno.writeTextFile(path.join(tempDir, "pixman/pixman/pixman-config.h"), `#ifndef PIXMAN_CONFIG_H
#define PIXMAN_CONFIG_H

#define PACKAGE
#define HAVE_PTHREADS 1

#endif //PIXMAN_CONFIG_H
`);

    await Deno.writeTextFile(path.join(tempDir, "pixman/pixman/pixman-private.h"),
        '#include <pixman-config.h>\n' + await Deno.readTextFile(path.join(tempDir, "pixman/pixman/pixman-private.h"))
    );

    console.log("copy to src");
    await copyFolderRecursive(path.join(tempDir, "cairo/src"), path.join(__dirname, "src"));
    await copyFolderRecursive(path.join(tempDir, "pixman/pixman"), path.join(__dirname, "src"));

    console.log("cleanup src");
    await removeFilesGlob("src/**/*.in");
    await removeFilesGlob("src/**/*.sh");
    await removeFilesGlob("src/**/*.awk");
    await removeFilesGlob("src/**/*.build");
    await removeFilesGlob("src/**/Makefile*");
    await removeFilesGlob("src/**/README");

    await rm(tempDir);
}

export default {clean, fetch};
