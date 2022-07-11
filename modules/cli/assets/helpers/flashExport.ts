import {executeAsync, makeDirs} from "../../utils.ts";
import {getBinaryPath} from "../../utility/bin.ts";
import {path} from "../../../deps.ts";

export function flashExportAsync(configPath: string): Promise<number> {
    const bin = getBinaryPath("@ekx/ekx/tools/flash-export", "flash-export");
    return executeAsync(bin, ["export", configPath]);
}

export interface RenderFlashSymbolOutputOptions {
    scale: number;
    width: number;
    height: number;
    alpha: boolean;
    trim: boolean;
    outFilePath: string;
}

export function renderFlashSymbol(fla: string, symbol: string, outputs: RenderFlashSymbolOutputOptions[]) {
    const cmd = ["render", fla, symbol];
    for (const output of outputs) {
        cmd.push(
            "" + output.scale,
            "" + (output.width | 0),
            "" + (output.height | 0),
            "" + (output.alpha as any | 0),
            "" + (output.trim as any | 0),
            output.outFilePath
        );
        makeDirs(path.dirname(output.outFilePath));
    }
    const bin = getBinaryPath("@ekx/ekx/tools/flash-export", "flash-export");
    return executeAsync(bin, cmd);
}