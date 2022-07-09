import {executeAsync} from "../utils.ts";
import {fs} from "../../deps.ts";
import {logger} from "../logger.ts";

export function fixMP3(pattern: string) {
    const tasks: Promise<number>[] = [];
    const files = fs.expandGlobSync(pattern);
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
                Deno.removeSync(filepath);
                Deno.renameSync(f, filepath);
                return _;
            })
        );
    }
    return Promise.all(tasks);
}
