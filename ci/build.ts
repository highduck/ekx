import {existsSync} from "fs";
import {UtilityConfig} from "../modules/cli/utils.js";
import {logger} from "../modules/cli/logger.js";

// run build files
import "../packages/texture-loader/build.js";
import "../packages/auph/build.js";
import "../packages/graphics/build.js";
import "../plugins/firebase/build.js";

UtilityConfig.verbose = true;

// check files are really exists
const filesToVerify = [
    "packages/texture-loader/js/lib/lib-texture-loader.js",
    "packages/texture-loader/js/pre/texture-loader.js",
    "packages/auph/web/dist/emscripten/auph.js",
    "packages/auph/web/dist/emscripten/auph.js.map",
];

let missFiles = 0;
for (const file of filesToVerify) {
    if (existsSync(file)) {
        logger.info("✅ file is in place", file)
    } else {
        logger.warn("🤷 file is missing", file);
        ++missFiles;
    }
}

process.exit(missFiles > 0 ? 1 : 0);
