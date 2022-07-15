import {getModuleDir} from "../../modules/utils/utils.ts";
import {path} from "../../modules/deps.ts";
import * as esbuild from "https://deno.land/x/esbuild/mod.js";

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
}).catch(err => console.error(err));

console.info("auph build completed");
