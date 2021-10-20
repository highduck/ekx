import * as path from "path";
import {executeAsync} from "../../utils";
import {logger} from "../../logger";

export class WebpConfig {
    lossless = true;
    lossyQuality = 80;
    losslessQuality = 100;
}

export async function compress(filepath: string, config?: WebpConfig) {
    try {
        const ext = path.extname(filepath);
        const dest = filepath.replace(ext, ".webp");
        const options = [];
        if (config) {
            if (config.lossless) {
                options.push("-q", config.losslessQuality | 0, "-m", "6", "-lossless");
            } else {
                options.push("-q", config.lossyQuality | 0);
            }
        }
        logger.info([...options, filepath, "-o", dest].join(" "));
        await executeAsync("cwebp", [...options, filepath, "-o", dest]);
    } catch {
    }
}