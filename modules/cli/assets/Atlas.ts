import * as fs from "fs";
import * as path from "path";
import {compress, WebpConfig} from "./helpers/webp.js";
import {Asset, AssetDesc} from "./Asset.js";
import {spritePackerAsync} from "./helpers/spritePacker.js";
import {H} from "../utility/hash.js";
import {ensureDirSync, expandGlobSync, writeTextFileSync} from "../../utils/utils.js";

export interface MultiResAtlasImporterDesc extends AssetDesc {
    name: string; // required!
    resolutions?: MRAResolution[];
    webp?: WebpConfig; // {} by default (enabled)
}

export interface MRAResolution {
    scale: number;
    maxWidth?: number;// = 2048;
    maxHeight?: number;// = 2048;
}

export class MultiResAtlasAsset extends Asset {
    static typeName = "atlas_builder";

    inputs: string[] = [];

    constructor(readonly desc: MultiResAtlasImporterDesc) {
        super(desc, MultiResAtlasAsset.typeName);
    }

    async postBuild() {
        ensureDirSync(path.join(this.owner.cache, this.desc.name));
        const argsPath = path.join(this.owner.cache, this.desc.name, "args.txt");

        const resolutionNodes = [];
        for (const r of this.desc.resolutions!) {
            resolutionNodes.push(`${r.scale} ${r.maxWidth ?? 2048} ${r.maxHeight ?? 2048}`);
        }
// output_path
// atlas_name
// %u(num_resolutions)
// %f %u %u
// ..
// %u(num_inputs)
// input_path
// ...
        const args = `${this.owner.output}
${this.desc.name}
${resolutionNodes.length}
${resolutionNodes.join("\n")}
${this.inputs.length}
${this.inputs.join("\n")}
`;
        writeTextFileSync(argsPath, args);
        await spritePackerAsync(argsPath);

        if (this.owner.project.current_target === "ios") {
            this.desc.webp = undefined;
        }

        if (this.desc.webp) {
            const files = expandGlobSync(path.join(this.owner.output, `**/${this.desc.name}*.png`));
            const promises = [];
            for (const file of files) {
                promises.push(compress(file.path, this.desc.webp));
            }
            await Promise.all(promises);

            if (this.owner.project.current_target === "android") {
                for (const file of files) {
                    fs.rmSync(file.path);
                }
            }
        }

        this.writer.writeU32(H("atlas"));
        this.writer.writeString(this.desc.name);
        // variants
        let formatMask = 1;
        if (this.desc.webp) {
            if (this.owner.project.current_target === "android") {
                formatMask = 0;
            }
            formatMask |= 2;
        }
        this.writer.writeU32(formatMask);
    }
}