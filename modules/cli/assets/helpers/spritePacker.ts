import {execute} from "../../utils.js";
import {getOrBuildUtility} from "../../utility/bin.js";

export async function spritePacker(configPath: string): Promise<number> {
    const bin = await getOrBuildUtility("ekc");
    return await execute(bin, ["sprite-packer", configPath]);
}