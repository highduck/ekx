import * as fs from 'fs';
import * as path from 'path';
import * as Module from 'module';
import {logger} from "../logger";

// want to not have any dependencies, so just copy approach from `resolve-from` package
export function resolveFrom(fromDirectory: string, moduleId: string): string | null {
    try {
        fromDirectory = fs.realpathSync(fromDirectory);
    } catch (error) {
        if (error.code === 'ENOENT') {
            fromDirectory = path.resolve(fromDirectory);
        } else {
            return null;
        }
    }

    try {
        return resolveFileName(fromDirectory, moduleId);
    } catch (error) {
        logger.warn(`Cannot resolve module '${moduleId}' from '${fromDirectory}'`, error);
    }
    return null;
}

function resolveFileName(fromDirectory: string, moduleId: string): string {
    const fromFile = path.join(fromDirectory, 'noop.js');
    return (Module as any)._resolveFilename(moduleId, {
        id: fromFile,
        filename: fromFile,
        paths: (Module as any)._nodeModulePaths(fromDirectory)
    });
}