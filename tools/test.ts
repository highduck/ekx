import {shdc} from "../modules/sokol-shdc.ts";
import {getModuleDir, rm} from "../modules/utils/utils.ts";
import {fs, path} from "../modules/deps.ts";

const __dirname = getModuleDir(import.meta);

async function test_shdc() {
    await shdc({
        input: "test/simple2d.glsl",
        output: "test/simple2d_shader.h",
        cwd: __dirname
    });

    const hdr = path.join(__dirname, "test/simple2d_shader.h");
    if (!fs.existsSync(hdr)) {
        throw new Error("shader header not found");
    }

    await rm(hdr);
}

await test_shdc();