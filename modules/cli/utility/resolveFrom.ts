import {fs, path} from "../../deps.ts";
//import * as module from "https://deno.land/std/node/module.ts";
import * as module from "https://deno.land/std/node/module.ts";
import {logger} from "../logger.ts";

// want to not have any dependencies, so just copy approach from `resolve-from` package
export function resolveFrom(fromDirectory: string, moduleId: string): string | undefined {
    try {
        fromDirectory = Deno.realPathSync(fromDirectory);
    } catch (error) {
        logger.error(error);
        if (error.code === 'ENOENT') {
            fromDirectory = path.resolve(fromDirectory);
        } else {
            return undefined;
        }
    }

    try {
        return resolveFileName(fromDirectory, moduleId);
    } catch (error) {
        logger.debug(`Cannot resolve module '${moduleId}' from '${fromDirectory}'`, error);
    }
    return undefined;
}

function resolveFileName(fromDirectory: string, moduleId: string): string {
    const require = module.createRequire(fromDirectory);
    const Module = require("module");
    // console.info(Object.keys(req));
    // req.resolve("package.json");
    const fromFile = path.join(fromDirectory, 'noop.js');
    return Module._resolveFilename(moduleId, {
        id: fromFile,
        filename: fromFile,
        paths: Module._nodeModulePaths(fromDirectory)
    });
}