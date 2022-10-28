import * as fs from "fs";
import * as path from "path";
import {fileURLToPath} from 'url';
import {spawn} from "child_process";
import G from "glob";

/**
 * Remove directory, ignores errors
 */
export const rm = (filepath: string) => fs.promises.rm(filepath, {recursive: true}).catch(()=>undefined);

/**
 * Resolve `__dirname` like filepath for current module
 * @param meta module's import meta, pass `import.meta`
 */
export function getModuleDir(meta: ImportMeta): string {
    return path.dirname(fileURLToPath(meta.url));
}

export const ensureDirSync = (dirname: string) => {
    try {
        fs.mkdirSync(dirname, {recursive: true});
    } catch (e) {
    }
}

export function withPath<T>(dir: string, cb: () => T): T {
    const p = process.cwd();
    process.chdir(dir);
    const result = cb();
    process.chdir(p);
    return result;
}

export async function withPathAsync<T>(dir: string, cb: () => Promise<T>): Promise<T> {
    const p = process.cwd();
    process.chdir(dir);
    const result = await cb();
    process.chdir(p);
    return result;
}

export async function copyFolderRecursive(source: string, target: string) {
    ensureDirSync(target);

    if (fs.lstatSync(source).isDirectory()) {
        const entries = fs.readdirSync(source, {withFileTypes: true});
        for (const entry of entries) {
            const curSource = path.join(source, entry.name);
            const destSource = path.join(target, entry.name);
            if (entry.isDirectory()) {
                await copyFolderRecursive(curSource, destSource);
            } else {
                await fs.promises.copyFile(curSource, destSource);
            }
        }
    }
}

export const run = async (opts: { cmd: string[], cwd?: string, env?: Record<string, string>, stdio?: "inherit" | "ignore", io?: boolean }): Promise<{ success: boolean, code: number, data?: string, error?: string }> => {
    const [exe, ...args] = opts.cmd;
    const child = spawn(exe, args, {
        cwd: opts.cwd,
        env: opts.env,
        stdio: opts.stdio ?? "inherit",
    });
    let data: string | undefined, error: string | undefined;
    if (opts.io) {
        data = "";
        for await (const chunk of child.stdout) {
            //          console.log('stdout chunk: ' + chunk);
            data += chunk;
        }
        error = "";
        for await (const chunk of child.stderr) {
//            console.error('stderr chunk: ' + chunk);
            error += chunk;
        }
    }
    const code: number = await new Promise((resolve) => {
        child.on('close', resolve);
    });
    return {
        code,
        success: code === 0,
        data, error,
    };
}

export const writeTextFileSync = (filepath: URL | string, content: string) => {
    fs.writeFileSync(filepath, content, "utf8");
}

export const readTextFileSync = (filepath: URL | string): string => {
    return fs.readFileSync(filepath, "utf8");
}

export const expandGlobSync = (pattern: string, options?: { root?: string }): { path: string }[] => {
    let opts:any = undefined;//{root: process.cwd(), cwd: process.cwd(), absolute: true};
    if(options && options.root) {
        opts = {};
        opts.cwd = options.root;
        opts.root = options.root;
        opts.absolute = true;
    }
    const g = new G.GlobSync(pattern, opts);
    // console.info(g.found);
    return g.found.map(s=>({path:s}));
}