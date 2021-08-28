import {executeAsync} from "../utils";
import * as glob from 'glob';
import * as fs from "fs";
import {logger} from "../logger";

export function fixMP3(pattern: string) {
    const tasks: Promise<number>[] = [];
    const files = glob.sync(pattern);
    logger.info(pattern);
    for (const file of files) {
        logger.info(file);
        tasks.push(
            executeAsync("ffmpeg", [
                "-i", file,
                "-c:a", "copy",
                "-c:v", "copy",
                file.substring(0, file.length - 4) + "_.mp3"
            ]).then((_) => {
                const f = file.substring(0, file.length - 4) + "_.mp3"
                fs.rmSync(file);
                fs.renameSync(f, file);
                return _;
            })
        );
    }
    return Promise.all(tasks);
}
