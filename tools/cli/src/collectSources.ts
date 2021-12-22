import {searchFiles} from "./utils";
import {Project} from "./project";
import {ModuleDef, UserArray, VariableName} from "./module";
import * as path from "path";

export function collectSourceFiles(searchPath: string, extensions: string[], outList: string[] = []) {
    for (const ext of extensions) {
        searchFiles("**/*." + ext, searchPath, outList);
    }
    return outList;
}

// src_kind - "cpp", "java", "js", etc..
export function _collectObjects(data: any, variableName: VariableName, out: any[]) {
    if (data) {
        const list: UserArray<any> = data[variableName];
        if (list !== undefined) {
            if (list instanceof Array) {
                for (let obj of list) {
                    out.push(obj);
                }
            } else {
                out.push(list);
            }
        }
    }
}

type ModuleDefSource = Project | ModuleDef | ModuleDef[];

function extractModulesFromContext(ctx: ModuleDefSource): ModuleDef[] {
    return ctx instanceof Project ? ctx.modules : (ctx instanceof Array ? ctx : [ctx]);
}

export function collectCppFlags(ctx: ModuleDefSource, platforms: string[]): any[] {
    let all: any[] = [];
    const modules = extractModulesFromContext(ctx);
    for (const data of modules) {
        let result: any[] = [];
        _collectObjects(data, "cpp_flags", result);
        for (const platform of platforms) {
            _collectObjects(data[platform], "cpp_flags", result);
        }
        if (data.path) {
            for (const r of result) {
                r.files = r.files.map(f => path.resolve(data.path, f));
            }
        }
        all = all.concat(result);
    }
    return all;
}

export function collectObjects(ctx: ModuleDefSource, variableName: VariableName, platforms: string[]): any[] {
    let result: any[] = [];
    const modules = extractModulesFromContext(ctx);
    for (const data of modules) {
        _collectObjects(data, variableName, result);
        for (const platform of platforms) {
            _collectObjects(data[platform], variableName, result);
        }
    }
    return result;
}

// src_kind - "cpp", "java", "js", etc..
export function _collectStrings(data: any, variableName: VariableName, out: Set<string>, basePath?: string) {
    if (data && data[variableName]) {
        const list = data[variableName];
        if (list instanceof Array) {
            for (let v of list) {
                if (basePath) {
                    out.add(path.resolve(basePath, v));
                } else {
                    out.add(v);
                }
            }
        } else if (typeof list === "string") {
            if (basePath) {
                out.add(path.resolve(basePath, list));
            } else {
                out.add(list);
            }
        }
    }
}

export function collectStrings(ctx: ModuleDefSource, variableName: VariableName, platforms: string[], isPathVariable: boolean): string[] {
    let result = new Set<string>();
    let modules = extractModulesFromContext(ctx);
    for (const data of modules) {
        let basePath = isPathVariable ? data.path : undefined;
        _collectStrings(data, variableName, result, basePath);
        for (const platform of platforms) {
            _collectStrings(data[platform], variableName, result, basePath);
        }
    }
    return [...result];
}

// rel_to - optional, for example "." relative to current working directory
export function collectSourceRootsAll(ctx: ModuleDefSource, srcKind: VariableName, platforms: string[], relativeTo: string): string[] {
    let result = collectStrings(ctx, srcKind, platforms, true);
    if (relativeTo) {
        result = result.map(p => path.relative(relativeTo, p));
    }
    return result;
}