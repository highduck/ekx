import * as path from "path";
import {readTextFileSync, rm, writeTextFileSync} from "../../modules/utils/utils.js";
import {downloadFiles} from "../../modules/utils/download.js";
import {resolveEkxPath} from "../../modules/utils/cacheDir.js";
import {logger} from "../../modules/cli/index.js";

const destPath = resolveEkxPath("external/miniz");

async function clean() {
    await rm(path.join(destPath, "src"));
}

async function fetch() {
    await downloadFiles({
        srcBaseUrl: "https://github.com/richgel999/miniz/raw/master",
        destPath: path.join(destPath, "src"),
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

    writeTextFileSync(
        path.join(destPath, "src/miniz_export.h"),
        `
#ifndef MINIZ_EXPORT
#define MINIZ_EXPORT
#endif
`);

    const zipc = readTextFileSync(path.join(destPath, "src/miniz_zip.c"));
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

    writeTextFileSync(path.join(destPath, "src/miniz_zip.c"), zipc2);
    logger.info("miniz_zip.c patched");
}

export default {clean, fetch};