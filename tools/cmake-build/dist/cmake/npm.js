"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.readPkg = exports.collectDependencies = exports.dependencyBlock = exports.convertPackageName = exports.resolveFrom = void 0;
const fs = require("fs");
const path = require("path");
const Module = require("module");
// want to not have any dependencies, so just copy approach from `resolve-from` package
function resolveFrom(fromDirectory, moduleId) {
    try {
        fromDirectory = fs.realpathSync(fromDirectory);
    }
    catch (error) {
        if (error.code === 'ENOENT') {
            fromDirectory = path.resolve(fromDirectory);
        }
        else {
            return null;
        }
    }
    const fromFile = path.join(fromDirectory, 'noop.js');
    const resolveFileName = () => Module._resolveFilename(moduleId, {
        id: fromFile,
        filename: fromFile,
        paths: Module._nodeModulePaths(fromDirectory)
    });
    try {
        return resolveFileName();
    }
    catch (error) {
        return null;
    }
}
exports.resolveFrom = resolveFrom;
function convertPackageName(name) {
    if (name.length > 0 && name[0] === "@" && name.indexOf("/") > 0) {
        const parts = name.split("/");
        const scope = parts[0].substr(1);
        return scope + "::" + parts[1];
    }
    return name;
}
exports.convertPackageName = convertPackageName;
function dependencyBlock(name, dep, rel, dir) {
    return `# ${dep} => ${name}
if(NOT TARGET ${name})
    add_subdirectory(${rel} ${dir})
    message(STATUS "Add NPM module '${dep}' target '${name}' from directory: ${rel}")
else()
    message(STATUS "Skip NPM module '${dep}'. Target '${name}' already defined")
endif()`;
}
exports.dependencyBlock = dependencyBlock;
function collectDependencies(dependencies, output) {
    if (!dependencies) {
        return;
    }
    const convertBackslashes = process.platform === 'win32';
    for (const dep of Object.keys(dependencies)) {
        const cmakePath = resolveFrom(process.cwd(), dep + "/CMakeLists.txt");
        if (cmakePath != null) {
            const name = convertPackageName(dep);
            const dir = path.basename(dep);
            const where = path.dirname(cmakePath);
            let rel = path.relative(process.cwd(), where);
            if (convertBackslashes) {
                rel = rel.replace(/\\/g, "/");
            }
            output.push(dependencyBlock(name, dep, rel, dir));
        }
    }
}
exports.collectDependencies = collectDependencies;
function readPkg(dir) {
    // read current package.json
    let pkg = null;
    let p = "package.json";
    if (dir != null) {
        p = path.join(dir, p);
    }
    try {
        const text = fs.readFileSync(p, 'utf8');
        pkg = JSON.parse(text);
    }
    catch {
        console.error("error reading 'package.json' from ", p);
    }
    return pkg;
}
exports.readPkg = readPkg;
