import * as path from "path";
import {Project} from "./project";
import {execute, executeAsync} from "./utils";

export function ekc(ctx:Project, ...args:string[]) {
    const bin = path.join(ctx.path.EKX_ROOT, "dev-tools/bin/ekc");
    execute(bin, args);
}

export function ekcAsync(ctx:Project, ...args:string[]) {
    const bin = path.join(ctx.path.EKX_ROOT, "dev-tools/bin/ekc");
    return executeAsync(bin, args);
}