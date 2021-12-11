"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const npm_1 = require("./cmake/npm");
const fs = require("fs");
const path = require("path");
// read current package.json
const pkg = (0, npm_1.readPkg)(process.cwd());
if (pkg) {
    const cmakeModuleParts = [];
    const selfPkg = (0, npm_1.readPkg)(path.resolve(__dirname, ".."));
    if (selfPkg != null && selfPkg.version) {
        cmakeModuleParts.push(`message(STATUS "Link NPM projects: ${selfPkg.name}@${selfPkg.version}")`);
    }
    cmakeModuleParts.push(`message(STATUS "package: ${pkg.name}@${pkg.version}")`);
    cmakeModuleParts.push("# dependencies\n");
    (0, npm_1.collectDependencies)(pkg.dependencies, cmakeModuleParts);
    cmakeModuleParts.push("# devDependencies\n");
    (0, npm_1.collectDependencies)(pkg.devDependencies, cmakeModuleParts);
    fs.writeFileSync('npm.cmake', cmakeModuleParts.join('\n'));
}
else {
    // error
    process.exit(1);
}
