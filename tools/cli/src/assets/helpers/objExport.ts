import {executeAsync} from "../../utils";
import {getBinaryPath} from "../../utility/bin";

export function objExportAsync(input: string, output: string): Promise<number> {
    const bin = getBinaryPath("@ekx/obj-export", "obj-export");
    return executeAsync(bin, [input, output]);
}