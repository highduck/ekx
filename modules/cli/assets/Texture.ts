import * as path from "path";
import * as fs from "fs";
import {Asset, AssetDesc} from "./Asset.js";
import {compress, WebpConfig} from "./helpers/webp.js";
import {H} from "../utility/hash.js";
import {hashFile} from "./helpers/hash.js";
import {ensureDirSync} from "../../utils/utils.js";

export const enum TextureDataType {
    Normal = "2d",
    CubeMap = "cubemap"
}

export interface TextureImporterDesc extends AssetDesc {
    name: string;
    type?: TextureDataType; // Normal
    images: string[];
    webp?: WebpConfig;
}

export class TextureAsset extends Asset {
    static typeName = "texture";

    constructor(readonly desc: TextureImporterDesc) {
        super(desc, TextureAsset.typeName);
    }

    resolveInputs(): number {
        let hash = super.resolveInputs();
        for (const imagePath of this.desc.images) {
            hash ^= hashFile(path.join(this.owner.basePath, imagePath));
        }
        return hash;
    }

    async build() {
        if (this.owner.project.current_target === "ios") {
            this.desc.webp = undefined;
        }

        for (const imagePath of this.desc.images) {
            const srcFilePath = path.join(this.owner.basePath, imagePath);
            const destFilepath = path.join(this.owner.output, imagePath);
            ensureDirSync(path.dirname(destFilepath));
            fs.copyFileSync(srcFilePath, destFilepath);
        }
        if (this.desc.webp) {
            const promises = [];
            for (const imagePath of this.desc.images) {
                const filepath = path.join(this.owner.output, imagePath);
                promises.push(compress(filepath, this.desc.webp));
            }
            await Promise.all(promises);
            if (this.owner.project.current_target === "android") {
                for (const imagePath of this.desc.images) {
                    const filepath = path.join(this.owner.output, imagePath);
                    fs.rmSync(filepath);
                }
            }
        }

        this.writer.writeU32(H(this.typeName));

        // res-name
        this.writer.writeU32(H(this.desc.name));

        // texture data
        this.writer.writeU32(this.desc.type === TextureDataType.Normal ? 0 : 1);

        // variants
        let formatMask = 1;
        if (this.desc.webp) {
            if (this.owner.project.current_target === "android") {
                formatMask = 0;
            }
            formatMask |= 2;
        }
        this.writer.writeU32(formatMask);
        this.writer.writeFixedASCIIArray(this.desc.images, 128);
    }
}