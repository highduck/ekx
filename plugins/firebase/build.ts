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

await esbuild("web/firebase.js", "--global-name=firebase_js", "--bundle", "--sourcemap", "--target=chrome58,firefox57,safari11,edge16", "--outfile=web/dist/firebase.js");
await esbuild("web/firebase_c.js", "--bundle", "--format=esm", "--sourcemap", "--target=node14", "--outfile=web/lib/firebase_c.js");
