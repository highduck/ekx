const {downloadCheck, untar, copyFolderRecursiveSync} = require('@ekx/cli-utils');
const fs = require("fs");
const path = require("path");
const rimraf = require("rimraf");

async function downloadCairo() {
    const url = "https://www.cairographics.org/snapshots/cairo-1.17.4.tar.xz";
    const sha1 = "68712ae1039b114347be3b7200bc1c901d47a636";
    await downloadCheck(url, "_dist", sha1);
    await untar("_dist/" + path.basename(url), "_dist/cairo", {strip: 1});
}

async function downloadPixMan() {
    const url = "https://www.cairographics.org/snapshots/pixman-0.33.6.tar.gz";
    const sha1 = "f174d00517e7e1d81c90c65efc20dd876877d904";
    await downloadCheck(url, "_dist", sha1);
    await untar("_dist/" + path.basename(url), "_dist/pixman", {strip: 1});
}

async function run() {
    await Promise.all([downloadCairo(), downloadPixMan()]);

    fs.writeFileSync("_dist/cairo/src/cairo-features.h", `#ifndef CAIRO_FEATURES_H
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

    fs.writeFileSync("_dist/pixman/pixman/pixman-config.h", `#ifndef PIXMAN_CONFIG_H
#define PIXMAN_CONFIG_H

#define PACKAGE
#define HAVE_PTHREADS 1

#endif //PIXMAN_CONFIG_H
`);

    fs.writeFileSync("_dist/pixman/pixman/pixman-private.h",
        '#include <pixman-config.h>\n' + fs.readFileSync("_dist/pixman/pixman/pixman-private.h")
    );

    copyFolderRecursiveSync("_dist/cairo/src", "src");
    copyFolderRecursiveSync("_dist/pixman/pixman", "src");

    rimraf.sync("src/**/*.in");
    rimraf.sync("src/**/*.sh");
    rimraf.sync("src/**/*.awk");
    rimraf.sync("src/**/*.build");
    rimraf.sync("src/**/Makefile*");
    rimraf.sync("src/**/README");
}

run().catch(() => process.exit(1));