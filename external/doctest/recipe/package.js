const {downloadFiles} = require('@ekx/cli-utils');

async function run() {
    console.info("doctest");
    await downloadFiles({
        srcBaseUrl: "https://github.com/onqtam/doctest/raw/master",
        fileList: [
            "doctest/doctest.h"
        ]
    });
}

run().catch(() => process.exit(1));