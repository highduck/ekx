import {executeAsync, makeDirs} from "../utils";
import {getBinaryPath} from "../utility/bin";
import * as path from "path";

export function flashExportAsync(configPath: string): Promise<number> {
    const bin = getBinaryPath("@ekx/flash-export", "flash-export");
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
    const bin = getBinaryPath("@ekx/flash-export", "flash-export");
    return executeAsync(bin, cmd);
}