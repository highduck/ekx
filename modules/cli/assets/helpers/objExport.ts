import {executeAsync} from "../../utils.ts";
import {resolveToolsBinPath} from "../../utility/bin.ts";

export function objExportAsync(input: string, output: string): Promise<number> {
    const bin = resolveToolsBinPath("obj-export");
    return executeAsync(bin, [input, output]);
}