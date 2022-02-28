import {compress, WebpConfig} from "./helpers/webp";
import {Asset, AssetDesc} from "./Asset";
import {makeDirs} from "../utils";
import * as path from "path";
import * as fs from "fs";
import * as glob from "glob";
import {spritePackerAsync} from "./helpers/spritePacker";
import {H} from "../utility/hash";

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
        makeDirs(path.join(this.owner.cache, this.desc.name));
        const configPath = path.join(this.owner.cache, this.desc.name, "_config.xml");

        const resolutionNodes = [];
        for (const r of this.desc.resolutions) {
            resolutionNodes.push(`<resolution scale="${r.scale}" maxWidth="${r.maxWidth ?? 2048}" maxHeight="${r.maxHeight ?? 2048}"/>`);
        }
        const inputNodes = [];
        for (const i of this.inputs) {
            inputNodes.push(`<images path="${i}"/>`);
        }
        // path="${path.resolve(this.owner.basePath, this.resourcePath)}"
        const xml = `<atlas name="${this.desc.name}" output="${this.owner.output}">
    ${resolutionNodes.join("\n")}
    ${inputNodes.join("\n")}
</atlas>`;
        fs.writeFileSync(configPath, xml, "utf8");
        await spritePackerAsync(configPath);

        if (this.owner.project.current_target === "ios") {
            this.desc.webp = undefined;
        }

        if (this.desc.webp) {
            const files = glob.sync(path.join(this.owner.output, `**/${this.desc.name}*.png`));
            const promises = [];
            for (const filepath of files) {
                promises.push(compress(filepath, this.desc.webp));
            }
            await Promise.all(promises);

            if (this.owner.project.current_target === "android") {
                for (const filepath of files) {
                    fs.rmSync(filepath);
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