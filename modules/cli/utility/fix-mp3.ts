import * as fs from "fs";
import {executeAsync} from "../utils.js";
import {logger} from "../logger.js";
import {expandGlobSync} from "../../utils/utils.js";

export function fixMP3(pattern: string) {
    const tasks: Promise<number>[] = [];
    const files = expandGlobSync(pattern);
    logger.info(pattern);
    for (const file of files) {
        const filepath = file.path;
        logger.info(filepath);
        tasks.push(
            executeAsync("ffmpeg", [
                "-i", filepath,
                "-c:a", "copy",
                "-c:v", "copy",
                filepath.substring(0, filepath.length - 4) + "_.mp3"
            ]).then((_) => {
                const f = filepath.substring(0, filepath.length - 4) + "_.mp3"
                fs.rmSync(filepath);
                fs.renameSync(f, filepath);
                return _;
            })
        );
    }
    return Promise.all(tasks);
}
