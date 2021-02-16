import * as path from "path";
import {Project} from "./project";
import {execute} from "./utils";

export function ekc(ctx:Project, ...args:string[]) {
    const bin = path.join(ctx.path.EKX_ROOT, "dev-tools/bin/ekc");
    execute(bin, args);
}