import {getToolsBinPath} from "../modules/cli/utility/bin.ts";
import {download} from "../modules/utils/download.ts";
import {fs, path} from "../modules/deps.ts";

async function fetch_shdc() {
    // download shdc tool
    const exeURL = {
        linux: "bin/linux/sokol-shdc",
        darwin: "bin/osx/sokol-shdc",
        windows: "bin/win32/sokol-shdc.exe",
    }[Deno.build.os];
    const exePath = getToolsBinPath("sokol-shdc");
    fs.ensureDirSync(path.dirname(exePath));
    await download("https://github.com/floooh/sokol-tools-bin/raw/master/" + exeURL, exePath);
    Deno.chmodSync(exePath, 0o755);
}

await fetch_shdc();
