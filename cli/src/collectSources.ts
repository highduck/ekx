import {searchFiles} from "./utils";
import {Project, SourceKind} from "./project";
import * as path from "path";

export function collectSourceFiles(searchPath: string, extensions: string[], outList: string[] = []) {
    for (const ext of extensions) {
        searchFiles("**/*." + ext, searchPath, outList);
    }
    return outList;
}

// src_kind - "cpp", "java", "js", etc..
export function collectSourceRoots(data: any, srcKind: SourceKind) {
    const result = [];
    if (data && data[srcKind]) {
        const sources = data[srcKind];
        if(sources instanceof Array) {
            result.push(...sources);
        }
    }
    return result;
}

// rel_to - optional, for example "." relative to current working directory
export function collectSourceRootsAll(ctx: Project, srcKind: SourceKind, extraTarget: string, relativeTo: string) {
    let result = [];
    for (const data of ctx.modules) {
        result = result.concat(collectSourceRoots(data, srcKind));
        if (extraTarget) {
            result = result.concat(collectSourceRoots(data[extraTarget], srcKind));
        }
    }
    if (relativeTo) {
        result = result.map((p) => path.relative(relativeTo, p));
    }
    return result;
}
