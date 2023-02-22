import {build} from "./modules/cmake/mod.js";

// build tools
await build({
    test: true,
    debug: false
});
