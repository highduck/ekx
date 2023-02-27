import {logger} from "../cli/logger.js";
import {collectRoots, resetWorkspace} from "./ws.js";
import * as fs from "fs";
import {spawnSync} from "child_process";
import * as path from "path";

// updates lock files in multi repository layout space
// TODO: detect single layout from cwd
// TODO: use cwd and add to running via ekx command
export const updateLockFiles = async () => {
    const roots = collectRoots(".");

    // remove super package from roots
    roots.shift();

    logger.info("Root: backup package.json");
    fs.renameSync("./package.json", "./package-backup.json");

    logger.info("Root: delete node_modules and package-lock.json");
    resetWorkspace(".");

    for (const root of roots) {
        logger.info(root.dir + " : update package-lock.json");
        resetWorkspace(root.dir);
        try {
            spawnSync("npm", ["i"], {
                stdio: 'inherit',
                cwd: root.dir
            });
        } catch (err) {
            logger.warn("can't update lock at ", root.dir);
        }
    }

    logger.info("Root: restore initial package.json");
    fs.renameSync("./package-backup.json", "./package.json");

    logger.info("Remove workspaces node_modules");
    for (const root of roots) {
        try {
            fs.rmSync(path.join(root.dir, "node_modules"), {recursive: true});
            logger.info("removed node_modules");
        } catch {
            // ignore
        }
    }

    logger.info("Root: update MONOREPO package-lock.json and node_modules");
    try {
        spawnSync("npm", ["i"], {
            stdio: 'inherit',
        });
    } catch (err) {
        logger.warn("can't update lock at root");
    }
}