const {downloadFiles} = require('@ekx/cli-utils');

async function run() {
    console.info("pugixml");
    await downloadFiles({
        srcBaseUrl: "https://github.com/zeux/pugixml/raw/master",
        fileList: [
            "src/pugiconfig.hpp",
            "src/pugixml.hpp",
            "src/pugixml.cpp",
        ]
    });
}

run().catch(() => process.exit(1));