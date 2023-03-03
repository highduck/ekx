import {execute} from "../../utils.js";
import {getOrBuildUtility} from "../../utility/bin.js";

export async function bmfont(configPath: string): Promise<number> {
    const bin = await getOrBuildUtility("ekc");
    return await execute(bin, ["bmfont-export", configPath]);
}