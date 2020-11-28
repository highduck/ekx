import {spawnSync} from "child_process";
import * as fs from "fs";
import * as path from "path";
import * as glob from 'glob';

export function execute(cmd, args) {
    console.debug(">> " + [cmd].concat(args).join(" "));
    console.debug("cwd", process.cwd());
    const child = spawnSync(cmd, args, {
            stdio: 'pipe',
            encoding: 'utf-8',
            cwd: process.cwd()
        }
    );
    console.log("stderr", child.stderr.toString());
    console.log("stdout", child.stdout.toString());
    console.log("exit code", child.status);
    if (child.error) {
        console.error(child.error);
    }

    return child.status;
}

export function optimizePng(input: string, output?: string) {
    const pngquant = require('pngquant-bin');
    if (!output) output = input;
    const result = spawnSync(pngquant, [
        "--strip",
        "--force",
        "-o", output,
        input
    ]);
    if (result.status === 0) {
        console.log('Image minified! ' + input);
    } else {
        console.warn(result.stderr.toString());
        console.warn(result.status);
    }
}

export function optimizePngGlob(input_pattern: string) {
    const files = glob.sync(input_pattern);
    for (const file of files) {
        optimizePng(file, file);
    }
}

export function withPath(path: string, cb: () => void) {
    const p = process.cwd();
    process.chdir(path);
    cb();
    process.chdir(p);
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

export function replaceInFile(filepath: string, dict: { [key: string]: string }) {
    let text = readText(filepath);
    for (const [k, v] of Object.entries(dict)) {
        text = replaceAll(text, k, v);
    }
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