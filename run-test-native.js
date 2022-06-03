const {build} = require('cmake-build');

build({
    test: true,
    debug: true,
    definitions: {
        EKX_BUILD_TESTS: "ON",
        EKX_BUILD_COVERAGE: "ON"
    },
}).catch(_ => process.exit(1));