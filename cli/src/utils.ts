import {spawnSync} from "child_process";
import * as fs from "fs";
import * as path from "path";
import * as glob from 'glob';
// ekRequire: function (id) {
//     try {
//         return require(id);
//     } catch {
//     }
//     return require(`${project.path.EKX_ROOT}/cli/node_modules/${id}`);
// },

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

export function optimize_png(input: string, output?: string) {
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

export function optimize_png_glob(input_pattern: string) {
    const glob = require('glob');
    const files = glob.sync(input_pattern);
    for (const file of files) {
        optimize_png(file, file);
    }
}

export function with_path(path: string, cb: () => void) {
    const p = process.cwd();
    process.chdir(path);
    cb();
    process.chdir(p);
}


export function replace_all(str: string, search: string, replacement: string) {
    return str.split(search).join(replacement);
}

export function read_text(src: string) {
    return fs.readFileSync(src, "utf8");
}

export function write_text(filepath: string, text: string) {
    fs.writeFileSync(filepath, text, "utf8");
}

export function copy_file(src: string, dest: string) {
    fs.copyFileSync(src, dest);
}

export function is_dir(p: string) {
    return fs.existsSync(p) && fs.lstatSync(p).isDirectory();
}

export function is_file(p: string) {
    return fs.existsSync(p) && fs.lstatSync(p).isFile();
}

export function replace_in_file(filepath: string, dict: { [key: string]: string }) {
    let text = read_text(filepath);
    for (const [k, v] of Object.entries(dict)) {
        text = replace_all(text, k, v);
    }
    write_text(filepath, text);
}

export function make_dirs(p: string) {
    // todo: improve (relax node version < 11)
    if (!is_dir(p)) {
        fs.mkdirSync(p, {recursive: true});
    }
}

export function search_files(pattern: string, search_path: string, out_files_list: string[]) {
    const files = glob.sync(pattern, {
        cwd: search_path
    });
    for (let file of files) {
        out_files_list.push(path.join(search_path, file));
    }
}

export function copyFolderRecursiveSync(source: string, target: string) {
    make_dirs(target);

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
            var curPath = p + "/" + file;
            if (fs.lstatSync(curPath).isDirectory()) { // recurse
                deleteFolderRecursive(curPath);
            } else { // delete file
                fs.unlinkSync(curPath);
            }
        });
        fs.rmdirSync(p);
    }
}