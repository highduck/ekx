import {fs, path} from "../deps.ts";
import {logger} from "./logger.ts";

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

export async function executeAsync(bin: string, args: string[], options?: ExecuteOptions): Promise<number> {
    const cmd = [bin].concat(args);
    const env = Deno.env.toObject();
    const cwd = options?.workingDir ?? Deno.cwd();
    logger.log("run:", cmd.join(" "));
    const stdio = (options?.verbose ?? UtilityConfig.verbose) ? "inherit" : "null";
    const status = await Deno.run({
        cmd, cwd, env,
        stderr: stdio,
        stdout: stdio
    }).status();
    if (status.success) {
        return status.code;
    }
    throw new Error('Run failed! status code: ' + status.code + ", signal: " + status.signal);
}

export async function execute(cmd: string, args: string[], workingDir?: string, additionalEnvParams?: { [key: string]: string }): Promise<number> {
    const time = Date.now();
    logger.debug(">>", [cmd].concat(args).join(" "));
    const wd = workingDir ?? Deno.cwd();
    logger.debug(" | cwd:", wd);

    const options: Deno.RunOptions = {
        cmd: [cmd].concat(args),
        stderr: 'inherit',
        stdin: 'inherit',
        stdout: 'inherit',
        env: Object.assign({}, Deno.env.toObject())
    };

    if (workingDir) {
        options.cwd = workingDir;
    }

    if (additionalEnvParams) {
        options.env = Object.assign(options.env!, additionalEnvParams);
    }

    const status = await Deno.run(options).status();
    logger.log(" | time:", (Date.now() - time) / 1000, "ms");
    logger.log(" | exit code:", status.code);
    if (!status.success) {
        logger.error(status.signal);
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

export function withPath<T>(dir: string, cb: () => T): T {
    const p = Deno.cwd();
    Deno.chdir(dir);
    const result = cb();
    Deno.chdir(p);
    return result;
}

export function replaceAll(str: string, search: string, replacement: string) {
    return str.split(search).join(replacement);
}

export function readText(src: string) {
    return Deno.readTextFileSync(src);
}

export function writeText(filepath: string, text: string) {
    Deno.writeTextFileSync(filepath, text);
}

export function copyFile(src: string, dest: string) {
    Deno.copyFileSync(src, dest);
}

export function isDir(p: string) {
    return fs.existsSync(p) && Deno.lstatSync(p).isDirectory;
}

export function isFile(p: string) {
    return fs.existsSync(p) && Deno.lstatSync(p).isFile;
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
    fs.ensureDirSync(p);
    // if (!isDir(p)) {
    //     Deno.mkdirSync(p, {recursive: true});
    // }
}

export function searchFiles(pattern: string, search_path: string, out_files_list: string[]) {
    const from = Deno.realPathSync(search_path);
    logger.log(`Search "${pattern}" in ${search_path}`);
    for (const file of fs.expandGlobSync(pattern, {root: from})) {
        const rel = path.relative(from, file.path);
        out_files_list.push(path.join(search_path, rel));
    }
}

export function copyFolderRecursiveSync(source: string, target: string) {
    makeDirs(target);

    //copy
    if (Deno.lstatSync(source).isDirectory) {
        const list = Deno.readDirSync(source);
        for (const file of list) {
            const curSource = path.join(source, file.name);
            if (file.isDirectory) {
                copyFolderRecursiveSync(curSource, path.join(target, file.name));
            } else {
                Deno.copyFileSync(curSource, path.join(target, file.name));
            }
        }
    }
}

export function deleteFolderRecursive(p: string) {
    if (fs.existsSync(p)) {
        const list = Deno.readDirSync(p);
        for (const file of list) {
            const curPath = path.join(p, file.name);
            if (file.isDirectory) { // recurse
                deleteFolderRecursive(curPath);
            } else { // delete file
                Deno.removeSync(curPath);
            }
        }
        Deno.removeSync(p);
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
