import {rm} from "../modules/utils/utils.js";
import {UtilityConfig} from "../modules/cli/utils.js";

UtilityConfig.verbose = true;

await rm("plugins/firebase/web/dist");
await rm("plugins/firebase/web/lib");
await rm("packages/auph/web/dist");
await rm("packages/texture-loader/js");
await rm("build");