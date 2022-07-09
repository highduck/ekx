import {path} from "ekx/deps.ts"
import {downloadFiles, getModuleDir, rm} from "ekx/utils/mod.ts"
import {buildMatrix} from "ekx/cmake/mod.ts"

const __dirname = getModuleDir(import.meta);

async function clean() {
    await rm(path.join(__dirname, "build"));
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

async function test() {
    await buildMatrix({
        cmakePath: "test",
        test: true,
        workingDir: __dirname
    });
    await rm(path.join(__dirname, "build"));
}

export default {clean, fetch, test};
