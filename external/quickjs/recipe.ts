import * as path from "path";
import {downloadFiles, getModuleDir, rm} from "../../modules/utils/mod.js";

const __dirname = getModuleDir(import.meta);

async function clean() {
    await rm(path.join(__dirname, "src"));
}

async function fetch() {
    await downloadFiles({
        srcBaseUrl: "https://github.com/bellard/quickjs/raw/master",
        destPath: path.join(__dirname, "src"),
        fileList: [
            "cutils.c",
            "cutils.h",
            "libbf.c",
            "libbf.h",
            "libregexp-opcode.h",
            "libregexp.c",
            "libregexp.h",
            "libunicode-table.h",
            "libunicode.c",
            "libunicode.h",
            "list.h",
            // "qjs.c",
            // "qjsc.c",
            "quickjs-atom.h",
            "quickjs-libc.c",
            "quickjs-libc.h",
            "quickjs-opcode.h",
            "quickjs.c",
            "quickjs.h",
        ]
    });
}

export default {clean, fetch};
