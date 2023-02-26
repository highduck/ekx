import {join} from "path";
import {existsSync} from "fs";
import {shdc} from "../../modules/sokol-shdc.js";
import {getModuleDir, rm} from "../../modules/utils/utils.js";
import {UtilityConfig} from "../../modules/cli/index.js";

UtilityConfig.verbose = true;

const __dirname = getModuleDir(import.meta);

await shdc({
    input: "simple2d.glsl",
    output: "simple2d_shader.h",
    cwd: __dirname
});

const hdr = join(__dirname, "simple2d_shader.h");
if (!existsSync(hdr)) {
    throw new Error("shader header not found");
}

await rm(hdr);
