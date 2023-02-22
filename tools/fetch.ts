import * as fs from "fs";
import * as path from "path";
import * as os from "os";
import {getToolsBinPath} from "../modules/cli/utility/bin.js";
import {download} from "../modules/utils/download.js";
import {ensureDirSync} from "../modules/utils/utils.js";

async function fetch_shdc() {
    const platform = os.platform();
    // download shdc tool
    const exeURL = ({
        linux: "bin/linux/sokol-shdc",
        darwin: "bin/osx/sokol-shdc",
        win32: "bin/win32/sokol-shdc.exe",
    } as any)[platform];
    const exePath = getToolsBinPath("sokol-shdc");
    ensureDirSync(path.dirname(exePath));
    await download("https://github.com/floooh/sokol-tools-bin/raw/master/" + exeURL, exePath);
    if (platform !== "win32") {
        fs.chmodSync(exePath, 0o755);
    }
}

await fetch_shdc();
