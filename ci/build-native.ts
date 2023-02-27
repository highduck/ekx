import {build} from "../modules/cmake/mod.js";
import {UtilityConfig} from "../modules/cli/utils.js";

UtilityConfig.verbose = true;

// build main packages, plugins and tools
await build({
    definitions: {
        EKX_BUILD_DEV_TOOLS: "ON",
        EKX_BUILD_TESTS: "ON",
        EKX_BUILD_COVERAGE: "OFF",
        EKX_BUILD_EXTERNAL_TESTS: "OFF",
        EKX_INCLUDE_EXAMPLES: "ON",
        EKX_INCLUDE_PLUGINS: "ON",
    },
    test: true,
    debug: false
});
