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

await esbuild("web/src/webaudio/index.ts", "--global-name=auph", "--bundle", "--sourcemap", "--target=chrome58,firefox57,safari11,edge16", '--define:process.env.NODE_ENV="production"', "--outfile=web/dist/emscripten/auph.js");
