import {path} from "../../modules/deps.ts"
import {downloadFiles, getModuleDir, rm} from "../../modules/utils/mod.ts"

const __dirname = getModuleDir(import.meta);

async function clean() {
    await rm(path.join(__dirname, "src"));
}

async function fetch() {
    await downloadFiles({
        srcBaseUrl: "https://github.com/richgel999/miniz/raw/master",
        destPath: path.join(__dirname, "src"),
        fileList: [
            "miniz_common.h",
            "miniz.h",
            "miniz.c",
            "miniz_tinfl.h",
            "miniz_tinfl.c",
            "miniz_tdef.h",
            "miniz_tdef.c",
            "miniz_zip.h",
            "miniz_zip.c",
        ]
    });

    await Deno.writeTextFile(
        path.join(__dirname, "src/miniz_export.h"),
        `
#ifndef MINIZ_EXPORT
#define MINIZ_EXPORT
#endif
`);

    const zipc = await Deno.readTextFile(path.join(__dirname, "src/miniz_zip.c"));
    const zipc2 = zipc.replace(`cdir_ofs = MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_CDIR_OFS_OFS);`,
        `
    // patch
    cdir_ofs = MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_CDIR_OFS_OFS);
    if(cdir_ofs + cdir_size > pZip->m_archive_size) {
        cdir_size = pZip->m_archive_size - cdir_ofs;
    }
`);

    if (zipc2 === zipc) {
        throw new Error("Can't patch miniz_zip.c");
    }

    await Deno.writeTextFile(path.join(__dirname, "src/miniz_zip.c"), zipc2);
    console.info("miniz_zip.c patched");
}

export default {clean, fetch};