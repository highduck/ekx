import {getModuleDir} from "../../modules/utils/utils.js";
import * as path from "path";
import * as esbuild from "esbuild";
import {logger} from "../../modules/cli/logger.js";

const __dirname = getModuleDir(import.meta);
await esbuild.build({
    entryPoints: [path.resolve(__dirname, "web/firebase.js")],
    globalName: "firebase_js",
    target: ["chrome58", "firefox57", "safari11", "edge16"],
    bundle: true,
    sourcemap: true,
    outfile: path.join(__dirname, "web/dist/firebase.js")
}).catch(err => logger.error(err));

await esbuild.build({
    entryPoints: [path.resolve(__dirname, "web/firebase_c.js")],
    bundle: true,
    format: "esm",
    sourcemap: true,
    target: "node14",
    outfile: path.join(__dirname, "web/lib/firebase_c.js")
}).catch(err => logger.error(err));

logger.info("firebase build completed");
