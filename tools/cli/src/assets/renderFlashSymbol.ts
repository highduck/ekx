import * as path from "path";
import {makeDirs} from "../utils";
import {ekcAsync} from "./ekc";

export interface RenderFlashSymbolOutputOptions {
    scale: number;
    width: number;
    height: number;
    alpha: boolean;
    trim: boolean;
    outFilePath: string;
}

export function renderFlashSymbol(fla: string, symbol: string, outputs: RenderFlashSymbolOutputOptions[]) {
    const cmd = ["prerender_flash", fla, symbol];
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
    return ekcAsync(...cmd);
}