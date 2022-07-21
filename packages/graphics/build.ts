import {getModuleDir} from "../../modules/utils/utils.ts";
import {shdc} from "../../modules/sokol-shdc.ts";

await shdc({
    input: "src/ek/canvas.glsl",
    output: "src/ek/canvas_shader.h",
    cwd: getModuleDir(import.meta)
});

console.info("graphics build completed");