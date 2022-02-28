import {H} from "../../utility/hash";
import * as fs from "fs";
import * as glob from "glob";
import * as path from "path";
import {isFile} from "../../utils";

// if you modify seed value, you should also modify it in `@ekx/std` package: `ek/hash.h`
const seed = 0x811C9DC5;

// FNV-1a hash
export function hashObject(obj: any): number {
    return H(JSON.stringify(obj));
}

export function hashBytes(bytes: Uint8Array): number {
    let hash = seed;
    for (let i = 0; i < bytes.byteLength; ++i) {
        hash ^= bytes[i];
        hash += (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) + (hash << 24)
    }
    return hash;
}

export function hashFile(filepath: string): number {
    return hashBytes(fs.readFileSync(filepath));
}

export function hashGlob(pattern: string): number {
    let hash = 0;
    const files = glob.sync(pattern, {absolute: true});
    files.sort();
    for (const file of files) {
        if (isFile(file)) {
            hash ^= hashFile(path.join(file));
        }
    }
    return hash;
}
