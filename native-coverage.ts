import {build} from "./modules/cmake/mod.js";

await build({
    definitions: {
        EKX_BUILD_DEV_TOOLS: "OFF",
        EKX_BUILD_TESTS: "ON",
        EKX_BUILD_COVERAGE: "ON",
        EKX_BUILD_EXTERNAL_TESTS: "OFF",
        EKX_INCLUDE_EXAMPLES: "OFF",
        EKX_INCLUDE_PLUGINS: "OFF",
    },
    buildType: "Profiling",
    target: "coverage"
});
