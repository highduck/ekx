const {downloadFiles} = require('@ekx/cli-utils');

async function run() {
    console.info("quickjs");
    await downloadFiles({
        srcBaseUrl: "https://github.com/bellard/quickjs/raw/master",
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
        ],
        destPath:"src"
    });
}

run().catch(() => process.exit(1));