import {Asset, AssetDesc} from "./Asset.ts";
import {compress, WebpConfig} from "./helpers/webp.ts";
import {logger} from "../logger.ts";
import {path} from "../../deps.ts";
import {copyFile, makeDirs} from "../utils.ts";
import {H} from "../utility/hash.ts";
import {hashFile} from "./helpers/hash.ts";

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
            makeDirs(path.dirname(destFilepath));
            copyFile(srcFilePath, destFilepath);
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
                    Deno.removeSync(filepath);
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

        this.writer.writeU32(this.desc.images.length);
        for (const image of this.desc.images) {
            logger.assert(image.length < 128);
            this.writer.writeFixedASCII(image, 128);
        }
    }
}