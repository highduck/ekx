import * as fs from "fs";
import * as path from "path";
import {ensureDirSync, expandGlobSync, readTextFileSync, run, writeTextFileSync} from "../utils/utils.js";
import {logger} from "./logger.js";

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

export async function execute(bin: string, args: string[], options?: ExecuteOptions): Promise<number> {
    const cmd = [bin].concat(args);
    const env: Record<string, string | undefined> = process.env;
    const cwd = options?.workingDir ?? process.cwd();
    logger.log("run:", cmd.join(" "));
    const stdio = (options?.verbose ?? UtilityConfig.verbose) ? "inherit" : "ignore";
    const status = await run({
        cmd, cwd, env,
        stdio: stdio,
    });
    if (status.success) {
        return status.code;
    }
    throw new Error('Run failed! status code: ' + status.code);
}

export async function execute2(cmd: string, args: string[], workingDir?: string, additionalEnvParams?: { [key: string]: string }): Promise<number> {
    const time = Date.now();
    logger.debug(">>", [cmd].concat(args).join(" "));
    const wd = workingDir ?? process.cwd();
    logger.debug(" | cwd:", wd);

    const options: any = {
        cmd: [cmd].concat(args),
        stdio: 'inherit',
        env: Object.assign({}, process.env)
    };

    if (workingDir) {
        options.cwd = workingDir;
    }

    if (additionalEnvParams) {
        options.env = Object.assign(options.env!, additionalEnvParams);
    }

    const status = await run(options);
    logger.log(" | time:", (Date.now() - time) / 1000, "ms");
    logger.log(" | exit code:", status.code);
    if (!status.success) {
        logger.error(status.code);
    }
    return status.code;
}

// export function optimizePngGlobAsync(input_pattern: string): Promise<any> {
//     const pngquant = require("pngquant-bin");
//     const files = glob.sync(input_pattern);
//     const tasks: Promise<number>[] = [];
//     for (const file of files) {
//         tasks.push(executeAsync(pngquant, [
//             "--strip",
//             "--force",
//             "-o", file,
//             file
//         ]));
//     }
//     return Promise.all(tasks).catch((err) => {
//         logger.warn("Can't optimize PNG images:", err);
//     });
// }


export function replaceAll(str: string, search: string, replacement: string) {
    return str.split(search).join(replacement);
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
    const text = substituteAll(readTextFileSync(filepath), dict);
    writeTextFileSync(filepath, text);
}

export function searchFiles(pattern: string, search_path: string, out_files_list: string[]) {
    // const fromm = path.resolve(search_path);
    const from = fs.realpathSync(search_path);
    logger.log(`Search "${pattern}" in ${search_path} (${from})`);
    for (const file of expandGlobSync(pattern, {root: from})) {
        const rel = path.relative(from, file.path);
        out_files_list.push(path.join(search_path, rel));
    }
}

export function copyFolderRecursiveSync(source: string, target: string) {
    ensureDirSync(target);

    //copy
    if (fs.lstatSync(source).isDirectory()) {
        const list = fs.readdirSync(source, {withFileTypes: true});
        for (const file of list) {
            const curSource = path.join(source, file.name);
            if (file.isDirectory()) {
                copyFolderRecursiveSync(curSource, path.join(target, file.name));
            } else {
                fs.copyFileSync(curSource, path.join(target, file.name));
            }
        }
    }
}

export function deleteFolderRecursive(p: string) {
    if (fs.existsSync(p)) {
        const list = fs.readdirSync(p, {withFileTypes: true});
        for (const file of list) {
            const curPath = path.join(p, file.name);
            if (file.isDirectory()) { // recurse
                deleteFolderRecursive(curPath);
            } else { // delete file
                fs.rmSync(curPath);
            }
        }
        fs.rmSync(p);
    }
}

//
// export async function rimrafAsync(pattern: string, options?: rimraf.Options): Promise<void> {
//     for await (const file of fs.expandGlob(pattern, {root: __dirname})) {
//         await Deno.remove(file.path);
//     }
// }

export function removePathExtension(p: string): string {
    const ext = path.extname(p);
    if (ext.length > 0) {
        return p.substring(0, p.length - ext.length);
    }
    return p;
}
