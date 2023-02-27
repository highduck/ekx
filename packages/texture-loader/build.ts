import {getModuleDir} from "../../modules/utils/utils.js";
import * as path from "path";
import * as esbuild from "esbuild";
import {logger} from "../../modules/cli/logger.js";

const __dirname = getModuleDir(import.meta);

await esbuild.build({
    entryPoints: [path.resolve(__dirname, "web/src/index.ts")],
    globalName: "TextureLoader",
    target: ["chrome58", "firefox57", "safari11", "edge16"],
    bundle: true,
    sourcemap: true,
    outfile: path.join(__dirname, "js/pre/texture-loader.js")
}).catch(err=> logger.error(err));

await esbuild.build({
    entryPoints: [path.resolve(__dirname, "web/src/lib.ts")],
    bundle: true,
    format: "esm",
    sourcemap: true,
    target: "node14",
    outfile: path.join(__dirname, "js/lib/lib-texture-loader.js")
}).catch(err=> logger.error(err));

logger.info("texture-loader build completed");
