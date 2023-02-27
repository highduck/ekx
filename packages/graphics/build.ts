import {getModuleDir} from "../../modules/utils/utils.js";
import {shdc} from "../../modules/sokol-shdc.js";
import {logger} from "../../modules/cli/logger.js";

await shdc({
    input: "src/ek/canvas.glsl",
    output: "src/ek/canvas_shader.h",
    cwd: getModuleDir(import.meta)
});

logger.info("graphics build completed");