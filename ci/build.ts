import "../packages/texture-loader/build.js";
import "../packages/auph/build.js";
import "../packages/graphics/build.js";
import "../plugins/firebase/build.js";

// check files are really exists
import {existsSync} from "fs";

const filesToVerify = [
    "packages/texture-loader/js/lib/lib-texture-loader.js",
    "packages/texture-loader/js/pre/texture-loader.js",
    "packages/auph/web/dist/emscripten/auph.js",
    "packages/auph/web/dist/emscripten/auph.js.map",
];

let missFiles = 0;
for (const file of filesToVerify) {
    if (existsSync(file)) {
        console.info("✅ file is in place", file)
    }
    else {
        console.warn("🤷 file is missing", file);
        ++missFiles;
    }
}

process.exit(missFiles > 0 ? 1 : 0);
