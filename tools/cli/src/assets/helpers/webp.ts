import * as path from "path";
import {executeAsync} from "../../utils";
import {logger} from "../../logger";

export interface WebpConfig {
    lossless?:boolean;// = true;
    lossyQuality?:number;// = 80;
    losslessQuality?:number// = 100;
}

export async function compress(filepath: string, config?: WebpConfig) {
    try {
        const ext = path.extname(filepath);
        const dest = filepath.replace(ext, ".webp");
        const options = [];
        if (config) {
            if (config.lossless ?? true) {
                options.push("-q", (config.losslessQuality ?? 100) | 0, "-m", "6", "-lossless");
            } else {
                options.push("-q", (config.lossyQuality ?? 80) | 0);
            }
        }
        logger.info([...options, filepath, "-o", dest].join(" "));
        await executeAsync("cwebp", [...options, filepath, "-o", dest], {verbose: false});
    } catch {
    }
}