import * as path from "path";
import * as os from "os";
import {executeAsync} from "../../utils.js";
import {logger} from "../../logger.js";
import {tryResolveCachedBin} from "../../utility/bin.js";
import {download, untar} from "../../../utils/download.js";
import {resolveCachePath} from "../../../utils/dirs.js";
import {copyFileSync} from "fs";
import {ensureDirSync} from "../../../utils/utils.js";

export interface WebpConfig {
    lossless?: boolean;// = true;
    lossyQuality?: number;// = 80;
    losslessQuality?: number// = 100;
}

const getWebPUrl = (): string => {
    const platform = os.platform();
    const arch = os.arch();
    const version = "1.3.0";
    let postfix: string = "";
    let ext = "tar.gz";
    switch (platform) {
        case "linux":
            postfix = "linux-x86-64";
            break;
        case "darwin":
            if (arch === "arm64") {
                postfix = "mac-arm64";
            } else if (arch === "x64") {
                postfix = "mac-x86-64";
            }
            break;
        case "win32":
            postfix = "windows-x64";
            ext = "zip";
            break;
    }
    if (postfix) {
        return `https://storage.googleapis.com/downloads.webmproject.org/releases/webp/libwebp-${version}-${postfix}.${ext}`;
    }
    throw new Error("Invalid os-arch: " + platform + ", " + arch);
};

const downloadCWebP = async (exeFilePath: string) => {
    const url = getWebPUrl();
    const sourcesPath = resolveCachePath("external/webp/sources");
    const info = await download(url, resolveCachePath("external/webp/artifacts", path.basename(url)));
    await untar(info.filepath, sourcesPath, {strip: 1});

    // copy extracted binary to expected bin place
    const binPath = path.join(sourcesPath, "bin", path.basename(exeFilePath));
    ensureDirSync(path.dirname(exeFilePath));
    copyFileSync(binPath, exeFilePath);
};

export const getOrFetchCWebP = async (): Promise<string> => {
    return await tryResolveCachedBin("cwebp", async (bin, exePath) => {
        logger.info("Miss " + bin);
        logger.info("Download webp...");
        await downloadCWebP(exePath);
    });
};

export async function compress(filepath: string, config?: WebpConfig) {
    const cwebp = await getOrFetchCWebP();
    try {
        const ext = path.extname(filepath);
        const dest = filepath.replace(ext, ".webp");
        const options: string[] = [];
        if (config) {
            if (config.lossless ?? true) {
                options.push("-q", ((config.losslessQuality ?? 100) | 0).toString(), "-m", "6", "-lossless");
            } else {
                options.push("-q", ((config.lossyQuality ?? 80) | 0).toString());
            }
        }
        logger.info([...options, filepath, "-o", dest].join(" "));
        await executeAsync(cwebp, [...options, filepath, "-o", dest], {verbose: false});
    } catch {
        // ignore
    }
}