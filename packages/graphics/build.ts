import {getModuleDir} from "../../modules/utils/utils.ts";

const __dirname = getModuleDir(import.meta);

async function shdc(...args:string[]) {
    const js = "../../external/sokol-shdc/sokol-shdc.js";
    await Deno.run({
        cmd: ["node", js].concat(args),
        cwd: __dirname,
        stdout: "inherit",
        stderr: "inherit"
    }).status();
}

await shdc("-i", "src/ek/canvas.glsl", "-o", "src/ek/canvas_shader.h", "-l", "glsl330:glsl300es:glsl100:hlsl5:metal_ios:metal_sim:metal_macos", "--ifdef");
