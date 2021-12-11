import {https} from 'follow-redirects';
import * as fs from 'fs';
import * as path from 'path';
import {spawn} from "child_process";
import * as crypto from 'crypto';

export function makeDirs(dir: string) {
    if (!fs.existsSync(dir)) {
        fs.mkdirSync(dir, {recursive: true});
    }
}

export function ensureFileDir(dest: string) {
    makeDirs(path.dirname(dest));
}

export function copyFolderRecursiveSync(source: string, target: string) {
    makeDirs(target);

    if (fs.lstatSync(source).isDirectory()) {
        fs.readdirSync(source).forEach((file) => {
            const curSource = path.join(source, file);
            if (fs.lstatSync(curSource).isDirectory()) {
                copyFolderRecursiveSync(curSource, path.join(target, file));
            } else {
                fs.copyFileSync(curSource, path.join(target, file));
            }
        });
    }
}


function downloadFile(url: string, dest: string) {
    return new Promise<void>((resolve, reject) => {
        ensureFileDir(dest);
        const file = fs.createWriteStream(dest);
        console.info("download: " + url);
        https.get(url, function (response) {
            response.pipe(file);
            file.on('finish', () => {
                console.info("saved: ", dest);
                resolve();
            }).on('error', (e) => {
                console.error("file save error:", e);
                reject(e);
            })
        }).on('error', (e) => {
            console.error("https request error:", e);
            reject(e);
        });
    });
}

export interface DownloadOptions {
    srcBaseUrl: string;
    // destination path, `process.cwd` if not defined
    destPath?: string;
    fileMap?: { [key: string]: string };
    fileList?: string[];
}

export function downloadFiles(props: DownloadOptions) {
    const srcBaseUrl = props.srcBaseUrl;
    const destPath = props.destPath ?? process.cwd();
    const fileMap = props.fileMap ?? {};
    const fileList = props.fileList ?? [];

    const tasks = [];

    for (const src of Object.keys(fileMap)) {
        const dest = fileMap[src] ?? src;
        const destFilePath = path.join(destPath, dest);
        const url = path.join(srcBaseUrl, src);
        tasks.push(downloadFile(url, destFilePath));
    }

    for (const src of fileList) {
        const destFilePath = path.join(destPath, src);
        const url = path.join(srcBaseUrl, src);
        tasks.push(downloadFile(url, destFilePath));
    }

    return Promise.all(tasks);
}

export async function untar(archivePath:string, dest:string, options?:{strip?:number}):Promise<void> {
    const args = [];
    if(options && options.strip != null) {
        args.push(`--strip-components=${options.strip}`);
    }
    makeDirs(dest);
    try {
        const status = await executeAsync("tar", ["-x", "-f", archivePath, ...args, "-C", dest], {verbose: true, passExitCode: true});
        if(status !== 0) {
            console.error("untar exit code", status);
        }
    }
    catch(e) {
        console.error("untar failed", e);
        throw e;
    }
}

export async function downloadAndUnpackArtifact(url: string, destDir: string, options?:{strip?:number}) {
    const name = path.basename(url);
    const archivePath = path.join(destDir, name);
    await downloadFile(url, archivePath);
    await untar(archivePath, "./" + destDir, options);
    fs.rmSync(archivePath);
}

export async function downloadCheck(url: string, destDir: string, sha1: string) {
    const name = path.basename(url);
    const archivePath = path.join(destDir, name);
    if (fs.existsSync(archivePath)) {
        const file = fs.readFileSync(archivePath);
        const sha1sum = crypto.createHash('sha1').update(file).digest("hex");
        if (sha1sum === sha1) {
            console.info("Check SHA1 verified, skip downloading", name);
            return;
        }
    }
    await downloadFile(url, archivePath);
}

const UtilityConfig = {
    verbose: true
};

export type ExecuteOptions = {
    cwd?: string,
    verbose?: boolean,
    passExitCode?: boolean
};

export function executeAsync(bin: string, args: string[], options?: ExecuteOptions): Promise<number> {
    return new Promise((resolve, reject) => {
        const child = spawn(bin, args, {
            //detached: true,
            stdio: (options?.verbose ?? UtilityConfig.verbose) ? "inherit" : "ignore",
            cwd: options?.cwd
        });
        child.on('close', (code) => {
            if (code === 0 || !!(options?.passExitCode)) {
                resolve(code);
            } else {
                reject('exit code: ' + code);
            }
        });
    });
}