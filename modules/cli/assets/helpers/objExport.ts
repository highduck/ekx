import {executeAsync} from "../../utils.js";
import {getOrBuildUtility} from "../../utility/bin.js";

export async function objExportAsync(input: string, output: string): Promise<number> {
    const bin = await getOrBuildUtility("obj-export");
    return await executeAsync(bin, ["obj", input, output]);
}