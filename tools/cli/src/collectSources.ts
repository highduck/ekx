import {searchFiles} from "./utils";
import {Project, LegacySourceKind} from "./project";
import * as path from "path";

export function collectSourceFiles(searchPath: string, extensions: string[], outList: string[] = []) {
    for (const ext of extensions) {
        searchFiles("**/*." + ext, searchPath, outList);
    }
    return outList;
}

// src_kind - "cpp", "java", "js", etc..
export function _collectLists(data: any, variableName: LegacySourceKind, out:Set<string>) {
    if (data && data[variableName]) {
        const list = data[variableName];
        if (list instanceof Array) {
            for(let v of list) {
                out.add(v);
            }
        }
    }
}

export function collectLists(ctx: Project, variableName: LegacySourceKind, extraTarget: string): string[] {
    let result = new Set<string>()
    for (const data of ctx.modules) {
        _collectLists(data, variableName, result);
        if (extraTarget) {
            _collectLists(data[extraTarget], variableName, result);
        }
    }
    return [...result];
}

// rel_to - optional, for example "." relative to current working directory
export function collectSourceRootsAll(ctx: Project, srcKind: LegacySourceKind, extraTarget: string, relativeTo: string) {
    let result = collectLists(ctx, srcKind, extraTarget);
    if (relativeTo) {
        result = result.map((p) => path.relative(relativeTo, p));
    }
    return result;
}