import {execute} from "../../utils.js";
import {getOrBuildUtility} from "../../utility/bin.js";

export async function objExport(input: string, output: string): Promise<number> {
    const bin = await getOrBuildUtility("ekc");
    return await execute(bin, ["obj", input, output]);
}