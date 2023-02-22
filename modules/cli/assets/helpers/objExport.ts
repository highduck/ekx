import {executeAsync} from "../../utils.js";
import {resolveToolsBinPath} from "../../utility/bin.js";

export function objExportAsync(input: string, output: string): Promise<number> {
    const bin = resolveToolsBinPath("obj-export");
    return executeAsync(bin, [input, output]);
}