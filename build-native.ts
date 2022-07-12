import {build} from "./modules/cmake/mod.ts";

// build tools
await build({
    test: false,
    debug: false,
    definitions: {
        EKX_BUILD_DEV_TOOLS: "ON",
        EKX_BUILD_TESTS: "OFF",
        EKX_BUILD_COVERAGE: "OFF",
        EKX_BUILD_EXTERNAL_TESTS: "OFF",
    },
});

// test
// await build({
//     test: true,
//     debug: true,
//     definitions: {
//         EKX_BUILD_TESTS: "ON",
//         EKX_BUILD_COVERAGE: "ON"
//     },
// });
