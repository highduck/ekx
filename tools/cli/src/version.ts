import * as fs from "fs";
import * as path from "path";
import {logger} from "./logger";

const BUILD_INFO_HEADER_FILE = "src/config/BuildInfo.h";

export const VERSION_INDEX_MAJOR = 0;
export const VERSION_INDEX_MINOR = 1;
export const VERSION_INDEX_PATCH = 2;
export const VERSION_INDEX_CODE = 3;

export type VersionTuple = [number, number, number];

function parseVersion(str: string): VersionTuple {
    const v = /(\d+)\.(\d+)\.(\d+)/.exec(str);
    if (v.length < 4) {
        throw "Error parse version string";
    }
    return [parseInt(v[1]), parseInt(v[2]), parseInt(v[3])];
}

function incVersion(kind: number, versionInOut: VersionTuple) {
    if (kind == 2) {
        ++versionInOut[2];
    } else if (kind == 1) {
        ++versionInOut[1];
        versionInOut[2] = 0;
    } else if (kind == 0) {
        ++versionInOut[0];
        versionInOut[1] = 0;
        versionInOut[2] = 0;
    }
}

export function increaseProjectVersion(p: string, versionIndex: number = VERSION_INDEX_CODE) {
    let config = fs.readFileSync(path.join(p, "ek.js"), "utf-8");
    let reVersion = /version_name\s*=\s*"(\d+\.\d+\.\d+)";/g
    let versionMatch = reVersion.exec(config);
    logger.info("version string:", versionMatch[0]);
    const oldVersion = versionMatch[1];
    const ver = parseVersion(versionMatch[1]);
    logger.info("version:", ver);
    let reCode = /version_code\s*=\s*"(\d+)";/g
    let codeMatch = reCode.exec(config);
    let code = parseInt(codeMatch[1]);

    incVersion(versionIndex, ver);
    code++;

    config = config.replace(versionMatch[0], `version_name = "${ver[0]}.${ver[1]}.${ver[2]}";`);
    config = config.replace(codeMatch[0], `version_code = "${code}";`);

    // rewrite version config header
    fs.writeFileSync(path.join(p, "ek.js"), config);

    // TODO: maybe we could update build info header on project export!
    const newVersion = `${ver[0]}.${ver[1]}.${ver[2]}`;
    let appVersionHeader = fs.readFileSync(path.join(p, BUILD_INFO_HEADER_FILE), "utf-8");
    appVersionHeader = appVersionHeader.replace(/Name\s*=\s*"(\d+).(\d+).(\d+)";/g, `Name = "${newVersion}";`);
    appVersionHeader = appVersionHeader.replace(/Code\s*=\s*"(\d+)";/g, `Code = "${code}";`);
    fs.writeFileSync(path.join(p, BUILD_INFO_HEADER_FILE), appVersionHeader);

    if (newVersion !== oldVersion) {
        const pkgPath = path.join(p, "package.json");
        let pkg = fs.readFileSync(pkgPath, "utf8");
        pkg = pkg.replace(/"version"\s*:\s*"([\d.]+)"/, `"version": "${newVersion}"`);
        fs.writeFileSync(pkgPath, pkg, "utf8");
    }
}