import * as fs from "fs";
import * as path from "path";
import {logger} from "./logger";

export const enum BumpVersionFlag {
    Major = 1,
    Minor = 2,
    Patch = 3,
    BuildNumber = 4
}

const semVerRegex = /^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$/;

export class SemVer {
    constructor(public major: number,
                public minor: number,
                public patch: number,
                public prerelease?: string,
                // timestamp, build number, etc
                public metadata?: string) {

    }

    static parseBump(str?: string, def?: BumpVersionFlag): BumpVersionFlag | undefined {
        if (str) {
            const lcs = str.toLowerCase();
            switch (lcs) {
                case "major":
                    return BumpVersionFlag.Major | BumpVersionFlag.BuildNumber;
                case "minor":
                    return BumpVersionFlag.Minor | BumpVersionFlag.BuildNumber;
                case "patch":
                    return BumpVersionFlag.Patch | BumpVersionFlag.BuildNumber;
                case "build":
                    return BumpVersionFlag.BuildNumber;
            }
        }
        return def;
    }

    static parse(str: string): SemVer {
        const v = semVerRegex.exec(str);
        if (v == null) {
            throw new Error("invalid sem-ver: " + str);
        }
        return new SemVer(
            parseInt(v[1]),
            parseInt(v[2]),
            parseInt(v[3]),
            v[4], v[5]
        );
    }

    toString(): string {
        let result = `${this.major}.${this.minor}.${this.patch}`;
        if (this.prerelease) {
            result += "-" + this.prerelease;
        }
        if (this.metadata) {
            result += "+" + this.metadata;
        }
        return result;
    }

    buildNumber(): number {
        if (this.metadata) {
            return parseInt(this.metadata);
        }
        return 1;
    }

    bump(bumpMask: BumpVersionFlag): void {
        const verMask = bumpMask & 0x3;
        switch (verMask) {
            case BumpVersionFlag.Patch:
                ++this.patch;
                break;
            case BumpVersionFlag.Minor:
                ++this.minor;
                this.patch = 0;
                break;
            case BumpVersionFlag.Major:
                ++this.major;
                this.minor = 0;
                this.patch = 0;
                break;
        }
        if (bumpMask & BumpVersionFlag.BuildNumber) {
            this.metadata = "" + (this.buildNumber() + 1);
        }
    }

    shortName() {
        let result = "" + this.major;
        if (this.patch || this.minor) {
            result += "." + this.minor;
            if (this.patch) {
                result += "." + this.patch;
            }
        }
        return result;
    }

    name() {
        return `${this.major}.${this.minor}.${this.patch}`;
    }
}

export function bumpProjectVersion(p: string, bumpMask = BumpVersionFlag.BuildNumber):SemVer {
    const pkgFilePath = path.join(p, "package.json");
    let pkg = fs.readFileSync(pkgFilePath, "utf-8");
    let reVersion = /"version"\s*:\s*"([^"]*)"/g
    let versionMatch = reVersion.exec(pkg);
    if (versionMatch == null) {
        throw new Error(`can't find "version" in package.json`);
    }
    const ver = SemVer.parse(versionMatch[1]);
    ver.bump(bumpMask);
    // rewrite version config header
    pkg = pkg.replace(versionMatch[0], `"version": "${ver.toString()}"`);
    fs.writeFileSync(pkgFilePath, pkg);

    return ver;
}