#!/usr/bin/env -S npx ts-node

/**
 * Script updates workspaces (local packages) dependencies versions depended on the monorepo setup
 */
import * as fs from "fs";
import * as path from "path";
import {collectPackages} from "./ws.js";
import {spawnSync} from "child_process";
import {logger} from "../cli/index.js";

const packages = collectPackages(".");
const map:Record<string, string> = {};
for (const p of packages) {
    map[p.pkg.name] = p.pkg.version;
}

let dryRun = false;

function updateVersions(name:string, deps:Record<string, string>) {
    let dirty = false;
    if (deps) {
        for (const dep of Object.keys(deps)) {
            const depVer = deps[dep];
            if (depVer !== "*") {
                const monorepoVer = map[dep];
                if (monorepoVer && depVer !== monorepoVer) {
                    logger.warn(name + " should set " + dep + " : " + depVer + " => " + monorepoVer);
                    if (!dryRun) {
                        deps[dep] = monorepoVer;
                    }
                    dirty = true;
                }
            }
        }
    }
    return dirty;
}

let someDirty = false;
for (const p of packages) {
    logger.info("check " + p.pkg.name);
    const dirty =
        updateVersions(p.pkg.name, p.pkg.dependencies) ||
        updateVersions(p.pkg.name, p.pkg.devDependencies);
    if (dirty) {
        someDirty = true;
        if (!dryRun) {
            logger.info("save " + p.pkg.name + " package.json");
            fs.writeFileSync(path.join(p.dir, "package.json"), JSON.stringify(p.pkg, null, 2), "utf8");
        }
    }
}

if (someDirty) {
    // finally update super space
    logger.info("final step install monorepo");
    spawnSync("npm i", {
        stdio: 'inherit'
    });
}