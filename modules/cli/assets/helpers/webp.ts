import {path} from "../../../deps.ts";
import {executeAsync} from "../../utils.ts";
import {logger} from "../../logger.ts";

export interface WebpConfig {
    lossless?: boolean;// = true;
    lossyQuality?: number;// = 80;
    losslessQuality?: number// = 100;
}

export async function compress(filepath: string, config?: WebpConfig) {
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
        await executeAsync("cwebp", [...options, filepath, "-o", dest], {verbose: false});
    } catch {
        // ignore
    }
}