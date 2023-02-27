#!/usr/bin/env -S npx ts-node

import {spawnSync} from 'child_process';
import {readFileSync, writeFileSync} from 'fs';
import * as path from 'path';
import * as glob from 'glob';
import {logger} from "../cli/logger.js";

const rootPkgText = readFileSync('package.json', 'utf8');
const rootPkg = JSON.parse(rootPkgText);
const workspaces = [];
for (const pattern of rootPkg.workspaces.packages) {
    for (const p of glob.sync(path.join(pattern, "package.json"))) {
        workspaces.push(p);
    }
}

const changedPackages = [];

for (const workspace of workspaces) {
    const dir = path.dirname(workspace);
    const exitCode = spawnSync("git", ["diff", "--quiet", "--cached", dir]).status;
    logger.info("staged changes:", exitCode, dir);
    if (exitCode !== 0) {
        const pkgText = readFileSync(workspace, 'utf8');
        const pkg = JSON.parse(pkgText);
        logger.info("changed:", pkg.name);
        changedPackages.push(pkg.name);
    }
}

if (changedPackages.length > 0) {
    const lines = [];
    lines.push('---');
    lines.push(...changedPackages.map((s) => `"${s}": patch`));
    lines.push('---');
    lines.push('');
    lines.push('auto update');

    const changesetFile = '.changeset/that-s-auto-update-changes.md';
    writeFileSync(changesetFile, lines.join('\n'), 'utf8');
    spawnSync("git", ["add", changesetFile]);
}



