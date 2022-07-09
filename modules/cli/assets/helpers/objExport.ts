import {executeAsync} from "../../utils.ts";
import {getBinaryPath} from "../../utility/bin.ts";

export function objExportAsync(input: string, output: string): Promise<number> {
    const bin = getBinaryPath("@ekx/obj-export", "obj-export");
    return executeAsync(bin, [input, output]);
}