import {fs, path} from "../deps.ts"
import {Sha1} from "https://deno.land/std/hash/sha1.ts"

export interface DownloadInfo {
    url: string | URL;
    size: number;
    filepath: string;
}

/** Download file from url to the destination. */
export async function download(url: string | URL, filepath: string, options?: RequestInit): Promise<DownloadInfo> {
    const response = await fetch(url, options);
    if (response.status !== 200) {
        throw new Deno.errors.Http(`status ${response.status}-'${response.statusText}' received instead of 200`);
    }
    const buffer = await response.arrayBuffer();
    const bytes = new Uint8Array(buffer);
    fs.ensureDirSync(path.dirname(filepath));
    Deno.writeFileSync(filepath, bytes);
    return {url, filepath, size: buffer.byteLength};
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
    const destPath = props.destPath ?? Deno.cwd();
    const fileMap = props.fileMap ?? {};
    const fileList = props.fileList ?? [];

    const tasks = [];

    for (const src of Object.keys(fileMap)) {
        const dest = fileMap[src] ?? src;
        const destFilePath = path.join(destPath, dest);
        const url = path.join(srcBaseUrl, src);
        tasks.push(download(url, destFilePath));
    }

    for (const src of fileList) {
        const destFilePath = path.join(destPath, src);
        const url = path.join(srcBaseUrl, src);
        tasks.push(download(url, destFilePath));
    }

    return Promise.all(tasks);
}

export async function downloadCheck(url: string, destDir: string, sha1: string) {
    const name = path.basename(url);
    const archivePath = path.join(destDir, name);
    if (fs.existsSync(archivePath)) {
        const file = Deno.readFileSync(archivePath);
        const sha1sum = new Sha1().update(file).hex();
        console.log(`Found file ${path.basename(archivePath)}, SHA1: ${sha1sum}`);
        if (sha1sum === sha1) {
            console.info("Check SHA1 verified, skip downloading", name);
            return;
        }
    }
    await download(url, archivePath);
}

export async function untar(archivePath:string, dest:string, options?:{strip?:number}):Promise<void> {
    const args = [];
    if(options && options.strip != null) {
        args.push(`--strip-components=${options.strip}`);
    }
    fs.ensureDirSync(dest);
    try {
        const status = await Deno.run({cmd:["tar", "-x", "-f", archivePath, ...args, "-C", dest]}).status();
        if(!status.success) {
            console.error("untar failed, exit code:", status.code);
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
    await download(url, archivePath);
    await untar(archivePath, destDir, options);
    await Deno.remove(archivePath);
}
