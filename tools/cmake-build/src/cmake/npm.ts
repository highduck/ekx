import * as fs from "fs";
import * as path from "path";
import * as Module from "module";

// want to not have any dependencies, so just copy approach from `resolve-from` package
export function resolveFrom(fromDirectory: string, moduleId: string): string {
    try {
        fromDirectory = fs.realpathSync(fromDirectory);
    } catch (error) {
        if (error.code === 'ENOENT') {
            fromDirectory = path.resolve(fromDirectory);
        } else {
            return null;
        }
    }

    const fromFile = path.join(fromDirectory, 'noop.js');
    const resolveFileName = () => (Module as any)._resolveFilename(moduleId, {
        id: fromFile,
        filename: fromFile,
        paths: (Module as any)._nodeModulePaths(fromDirectory)
    });

    try {
        return resolveFileName();
    } catch (error) {
        return null;
    }
}

export function convertPackageName(name: string): string {
    if (name.length > 0 && name[0] === "@" && name.indexOf("/") > 0) {
        const parts = name.split("/");
        const scope = parts[0].substr(1);
        return scope + "::" + parts[1];
    }
    return name;
}

export function dependencyBlock(name: string, dep: string, rel: string, dir: string): string {
    return `# ${dep} => ${name}
if(NOT TARGET ${name})
    add_subdirectory(${rel} ${dir})
    message(STATUS "Add NPM module '${dep}' target '${name}' from directory: ${rel}")
else()
    message(STATUS "Skip NPM module '${dep}'. Target '${name}' already defined")
endif()`
}

export function collectDependencies(dependencies:{[key:string]:string}|undefined, output:string[]):void {
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

export interface Pkg {
    name:string;
    version:string;
    dependencies?:{[key:string]:string},
    devDependencies?:{[key:string]:string},
    peerDependencies?:{[key:string]:string}
}

export function readPkg(dir:string):null|Pkg {
    // read current package.json
    let pkg = null;
    let p = "package.json";
    if (dir != null) {
        p = path.join(dir, p);
    }
    try {
        const text = fs.readFileSync(p, 'utf8');
        pkg = JSON.parse(text);
    } catch {
        console.error("error reading 'package.json' from ", p);
    }
    return pkg;
}