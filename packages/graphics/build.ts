import {getModuleDir} from "../../modules/utils/utils.js";
import {shdc} from "../../modules/sokol-shdc.js";

await shdc({
    input: "src/ek/canvas.glsl",
    output: "src/ek/canvas_shader.h",
    cwd: getModuleDir(import.meta)
});

console.info("graphics build completed");