import * as path from "path";
import {rm} from "../../modules/utils/utils.js";
import {downloadFiles} from "../../modules/utils/download.js";
import {resolveEkxPath} from "../../modules/utils/dirs.js";

const destPath = resolveEkxPath("external/quickjs");

async function clean() {
    await rm(path.join(destPath, "src"));
}

async function fetch() {
    await downloadFiles({
        srcBaseUrl: "https://github.com/bellard/quickjs/raw/master",
        destPath: path.join(destPath, "src"),
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
