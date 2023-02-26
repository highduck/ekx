import * as fs from "fs";
import * as path from "path";
import {spawnSync} from "child_process";
import * as glob from "glob";
import {logger} from "../cli/index.js";

export function readPkg(dir:string) {
    const data = fs.readFileSync(path.join(dir, "package.json"), "utf8");
    return JSON.parse(data);
}

export function getWorkspaces(pkg:any, dir:string) {
    const result = {
        patterns: [] as string[],
        workspaces: [] as string[],
    };

    if (pkg && pkg.workspaces && pkg.workspaces.packages && pkg.private) {
        for (const pattern of pkg.workspaces.packages) {
            result.patterns.push(path.join(dir, pattern));
            const packages = glob.sync(path.join(dir, pattern, "package.json"));
            for (const workspace of packages) {
                result.workspaces.push(path.dirname(workspace));
            }
        }
    }

    return result;
}

export function readWorkspaces(dir:string) {
    const pkg = readPkg(dir);
    return getWorkspaces(pkg, dir);
}

// UPGRADE functions

function getAllLocalPackageVersions(packages:{pkg:any, dir: string}[]) {
    const map:Record<string, string> = {};
    // detect local packages versions
    for (const pack of packages) {
        if (!pack.pkg.private && pack.pkg.version !== "0.0.0") {
            map[pack.pkg.name] = pack.pkg.version;
        }
    }
    return map;
}

function upgradePackage(localVersions:Record<string, string>, deps:Record<string, string>, kind:'dev'|any, cwd:string, ...customFlags:string[]) {
    if (!deps) {
        return;
    }

    const flags = ["--ignore-scripts", ...customFlags];
    if (kind === 'dev') {
        flags.push("--dev");
    }
    const toAdd = [];
    const toUpdate = [];
    for (const id of Object.keys(deps)) {
        const ver = localVersions[id];
        if (deps[id] === "*" || deps[id] === ver) {
            // skip
        } else if (ver) {
            toAdd.push(id + "@" + ver);
        } else {
            toUpdate.push(id);
        }
    }
    if (toAdd.length > 0) {
        logger.info("to add deps:", cwd, " - ", ...toAdd);
        spawnSync("yarn", ["add", ...toAdd, ...flags, "--exact"], {
            stdio: 'inherit',
            cwd: cwd
        });
    }
    if (toUpdate.length > 0) {
        logger.info("to update deps:", cwd, " - ", ...toUpdate);
        spawnSync("yarn", ["add", ...toUpdate, ...flags], {
            stdio: 'inherit',
            cwd: cwd
        });
    }
}

// you need detach super workspace before ru
export function upgradePackages(packages:{pkg:any, dir: string}[]) {
    const localMap = getAllLocalPackageVersions(packages);
    for (const pack of packages) {
        upgradePackage(localMap, pack.pkg.dependencies, "prod", pack.dir); //"--ignore-workspace-root-check"
        upgradePackage(localMap, pack.pkg.devDependencies, "dev", pack.dir);
    }
}

export const resetWorkspace = (dir:string) => {
    try {
        fs.rmSync(path.join(dir, "node_modules"), {recursive: true});
        logger.info("removed node_modules");
    } catch {/*ignore*/
    }
    try {
        fs.rmSync(path.join(dir, "package-lock.json"));
        logger.info("removed package-lock.json");
    } catch {/*ignore*/
    }
}

export function collectPackages(dir:string) {
    const packages:any[] = [];
    const pkg = readPkg(dir);
    if (pkg) {
        packages.push({
            pkg,
            dir
        });
        if (pkg.private) {
            if (pkg.projects) {
                for (const root of pkg.projects) {
                    packages.push(...collectPackages(path.join(dir, root)));
                }
            } else if (pkg.workspaces && pkg.workspaces.packages) {
                const {workspaces} = getWorkspaces(pkg, dir);
                for (const ws of workspaces) {
                    logger.info("collect workspace:", ws);
                    packages.push(...collectPackages(ws));
                }
            }
        }
    }
    return packages;
}

export function collectRoots(dir:string) {
    const roots:{pkg:any, dir: string}[] = [];
    const pkg = readPkg(dir);
    if (pkg) {
        roots.push({
            pkg,
            dir
        });
        if (pkg.private && pkg.projects) {
            for (const root of pkg.projects) {
                roots.push(...collectRoots(path.join(dir, root)));
            }
        }
    }
    return roots;
}
