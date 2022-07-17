import {build} from "./modules/cmake/mod.ts";

// build tools
await build({
    test: true,
    debug: false
});
