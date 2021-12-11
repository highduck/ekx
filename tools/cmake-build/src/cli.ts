import {collectDependencies, readPkg} from "./cmake/npm"
import * as fs from "fs";
import * as path from "path";

// read current package.json
const pkg = readPkg(process.cwd());
if(pkg) {
    const cmakeModuleParts = [];

    const selfPkg = readPkg(path.resolve(__dirname, ".."));
    if (selfPkg != null && selfPkg.version) {
        cmakeModuleParts.push(`message(STATUS "Link NPM projects: ${selfPkg.name}@${selfPkg.version}")`);
    }

    cmakeModuleParts.push(`message(STATUS "package: ${pkg.name}@${pkg.version}")`);

    cmakeModuleParts.push("# dependencies\n");
    collectDependencies(pkg.dependencies, cmakeModuleParts);

    cmakeModuleParts.push("# devDependencies\n");
    collectDependencies(pkg.devDependencies, cmakeModuleParts);

    fs.writeFileSync('npm.cmake', cmakeModuleParts.join('\n'));
}
else {
    // error
    process.exit(1);
}