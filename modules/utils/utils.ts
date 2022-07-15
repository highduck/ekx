import {path, fs} from "../deps.ts";

/**
 * Remove directory, ignores errors
 */
export function rm(filepath: string): Promise<void> {
    return Deno.remove(filepath, {recursive: true}).catch(() => undefined);
}

/**
 * Resolve `__dirname` like filepath for current module
 * @param meta module's import meta, pass `import.meta`
 */
export function getModuleDir(meta: ImportMeta): string {
    return path.dirname(path.fromFileUrl(meta.url));
}

export async function copyFolderRecursive(source: string, target: string) {
    await fs.ensureDir(target);

    if ((await Deno.lstat(source)).isDirectory) {
        for await (const entry of Deno.readDir(source)) {
            const curSource = path.join(source, entry.name);
            if (entry.isDirectory) {
                await copyFolderRecursive(curSource, path.join(target, entry.name));
            } else {
                await Deno.copyFile(curSource, path.join(target, entry.name));
            }
        }
    }
}

