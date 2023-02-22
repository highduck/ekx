import {executeAsync} from "../../utils.js";
import {getOrBuildToolBinary} from "../../utility/bin.js";

export async function objExportAsync(input: string, output: string): Promise<number> {
    const bin = await getOrBuildToolBinary("obj-export");
    return await executeAsync(bin, [input, output]);
}