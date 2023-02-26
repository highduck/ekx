import * as fs from "fs";
import * as Module from "module";
import * as path from "path";
import {logger} from "../logger.js";

// want to not have any dependencies, so just copy approach from `resolve-from` package
export function resolveFrom(fromDirectory: string, moduleId: string): string | undefined {
    try {
        fromDirectory = fs.realpathSync(fromDirectory);
    } catch (error) {
        logger.error(error);
        if (error instanceof Error && (error as any).code === 'ENOENT') {
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
    // const require = import.createRequire(fromDirectory);
    // const Module = require("module");
    // console.info(Object.keys(req));
    // req.resolve("package.json");
    const fromFile = path.join(fromDirectory, 'noop.js');
    return (Module as any)._resolveFilename(moduleId, {
        id: fromFile,
        filename: fromFile,
        paths: (Module as any)._nodeModulePaths(fromDirectory)
    });
}