import {getModuleDir} from "../../modules/utils/utils.js";
import * as path from "path";
import * as esbuild from "esbuild";
import {logger} from "../../modules/cli/logger.js";

const __dirname = getModuleDir(import.meta);

await esbuild.build({
    entryPoints: [path.resolve(__dirname, "web/src/webaudio/index.ts")],
    globalName: "auph",
    target: ["chrome58", "firefox57", "safari11", "edge16"],
    bundle: true,
    sourcemap: true,
    outfile: path.join(__dirname, "web/dist/emscripten/auph.js"),
    define: {
        "process.env.NODE_ENV":'"production"'
    }
}).catch(err => logger.error(err));

logger.info("auph build completed");
