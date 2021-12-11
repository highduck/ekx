const {downloadFiles} = require('@ekx/cli-utils');
const fs = require("fs");

async function run() {
    console.info("miniz");
    await downloadFiles({
        srcBaseUrl: "https://github.com/richgel999/miniz/raw/master",
        destPath: "src",
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

    await fs.promises.writeFile("src/miniz_export.h", `
#ifndef MINIZ_EXPORT
#define MINIZ_EXPORT
#endif
`);

    let miniz_zip = fs.readFileSync("src/miniz_zip.c", "utf8");
    const miniz_zip_patched = miniz_zip.replace(`cdir_ofs = MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_CDIR_OFS_OFS);`,
        `
    // patch
    cdir_ofs = MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_CDIR_OFS_OFS);
    if(cdir_ofs + cdir_size > pZip->m_archive_size) {
        cdir_size = pZip->m_archive_size - cdir_ofs;
    }
`);

    if (miniz_zip_patched === miniz_zip) {
        console.error("Can't patch miniz_zip.c");
        process.exit(-2);
    }

    fs.writeFileSync("src/miniz_zip.c", miniz_zip_patched, "utf8");

    console.info("miniz_zip.c patched");
    process.exit(0);
}

run().catch(() => process.exit(1));