import {getModuleDir} from "../../modules/utils/utils.ts";

const __dirname = getModuleDir(import.meta);

async function esbuild(...args) {
    await Deno.run({
        cmd: ["esbuild"].concat(args),
        cwd: __dirname,
        stdout: "inherit",
        stderr: "inherit"
    }).status();
}

await esbuild("web/src/index.ts", "--global-name=TextureLoader", "--bundle", "--sourcemap", "--target=chrome58,firefox57,safari11,edge16", "--outfile=js/pre/texture-loader.js");
await esbuild("web/src/lib.ts", "--bundle", "--format=esm", "--sourcemap", "--target=node14", "--outfile=js/lib/lib-texture-loader.js");
