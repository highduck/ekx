import {spawn, spawnSync, SpawnSyncOptionsWithStringEncoding} from "child_process";
import * as fs from "fs";
import * as path from "path";
import * as glob from 'glob';
import rimraf = require("rimraf");

export type UtilityConfig = {
    verbose?: boolean
};

export const UtilityConfig: UtilityConfig = {
    verbose: false
};

export type ExecuteOptions = {
    workingDir?: string,
    verbose?: boolean
};

export function executeAsync(bin: string, args: string[], options?: ExecuteOptions): Promise<number> {
    return new Promise((resolve, reject) => {
        const child = spawn(bin, args, {
            //detached: true,
            stdio: (options?.verbose ?? UtilityConfig.verbose) ? "inherit" : "ignore",
            cwd: options?.workingDir,
            env: process.env
        });
        child.on('close', (code) => {
            if (code === 0) {
                resolve(code);
            } else {
                reject('exit code: ' + code);
            }
        });
        child.on("error", (err) => {
            reject(err);
        });
    });
}

export function execute(cmd: string, args: string[], workingDir?: string, additionalEnvParams?: { [key: string]: string }): number {
    const time = Date.now();
    console.debug(">>", [cmd].concat(args).join(" "));
    const wd = workingDir ?? process.cwd();
    console.debug(" | cwd:", wd);

    const options: SpawnSyncOptionsWithStringEncoding = {
        stdio: 'inherit',
        encoding: 'utf-8',
        env: Object.assign({}, process.env)
    };

    if (workingDir) {
        options.cwd = workingDir;
    }

    if (additionalEnvParams) {
        Object.assign(options.env, additionalEnvParams);
    }

    const child = spawnSync(cmd, args, options);
    console.log(" | time:", (Date.now() - time) / 1000, "ms");
    console.log(" | exit code:", child.status);
    if (child.error) {
        console.error(child.error);
    }
    return child.status;
}

export async function optimizePngGlobAsync(input_pattern: string): Promise<any> {
    const pngquant = require("pngquant-bin");
    const files = glob.sync(input_pattern);
    const tasks: Promise<number>[] = [];
    for (const file of files) {
        tasks.push(executeAsync(pngquant, [
            "--strip",
            "--force",
            "-o", file,
            file
        ]));
    }
    return Promise.all(tasks).catch((err) => {
        console.warn("Can't optimize PNG images:", err);
    });
}

export function withPath<T>(dir: string, cb: () => T): T {
    const p = process.cwd();
    process.chdir(dir);
    const result = cb();
    process.chdir(p);
    return result;
}

export function replaceAll(str: string, search: string, replacement: string) {
    return str.split(search).join(replacement);
}

export function readText(src: string) {
    return fs.readFileSync(src, "utf8");
}

export function writeText(filepath: string, text: string) {
    fs.writeFileSync(filepath, text, "utf8");
}

export function copyFile(src: string, dest: string) {
    fs.copyFileSync(src, dest);
}

export function isDir(p: string) {
    return fs.existsSync(p) && fs.lstatSync(p).isDirectory();
}

export function isFile(p: string) {
    return fs.existsSync(p) && fs.lstatSync(p).isFile();
}

export function substituteAll(contents: string, dict: { [key: string]: string }): string {
    for (const [k, v] of Object.entries(dict)) {
        contents = replaceAll(contents, k, v);
    }
    return contents;
}

export function replaceInFile(filepath: string, dict: { [key: string]: string }) {
    const text = substituteAll(readText(filepath), dict);
    writeText(filepath, text);
}

export function makeDirs(p: string) {
    // todo: improve (relax node version < 11)
    if (!isDir(p)) {
        fs.mkdirSync(p, {recursive: true});
    }
}

export function searchFiles(pattern: string, search_path: string, out_files_list: string[]) {
    const files = glob.sync(pattern, {
        cwd: search_path
    });
    for (let file of files) {
        out_files_list.push(path.join(search_path, file));
    }
}

export function copyFolderRecursiveSync(source: string, target: string) {
    makeDirs(target);

    //copy
    if (fs.lstatSync(source).isDirectory()) {
        fs.readdirSync(source).forEach((file) => {
            var curSource = path.join(source, file);
            if (fs.lstatSync(curSource).isDirectory()) {
                copyFolderRecursiveSync(curSource, path.join(target, file));
            } else {
                fs.copyFileSync(curSource, path.join(target, file));
            }
        });
    }
}

export function deleteFolderRecursive(p: string) {
    if (fs.existsSync(p)) {
        fs.readdirSync(p).forEach((file, index) => {
            const curPath = p + "/" + file;
            if (fs.lstatSync(curPath).isDirectory()) { // recurse
                deleteFolderRecursive(curPath);
            } else { // delete file
                fs.unlinkSync(curPath);
            }
        });
        fs.rmdirSync(p);
    }
}

export function rimrafAsync(pattern: string, options?: rimraf.Options): Promise<void> {
    return new Promise((resolve, reject) => {
        const cb = (err) => {
            if (err) {
                reject(err);
            } else {
                resolve();
            }
        };
        if (options) {
            rimraf(pattern, options, cb);
        } else {
            rimraf(pattern, cb);
        }
    });
}