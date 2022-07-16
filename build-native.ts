import {build} from "./modules/cmake/mod.ts";

// build tools
await build({
    test: true,
    debug: false,
    os: "windows",
    definitions: {
        EKX_BUILD_DEV_TOOLS: "ON",
        EKX_BUILD_TESTS: "ON",
        EKX_BUILD_COVERAGE: "ON",
        EKX_BUILD_EXTERNAL_TESTS: "ON",
        EKX_INCLUDE_EXAMPLES: "ON",
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
