import {Asset} from "./Asset";
import {compress, WebpConfig} from "./helpers/webp";
import {logger} from "../logger";
import * as path from "path";
import {copyFile, makeDirs} from "../utils";
import * as fs from "fs";
import {BytesWriter} from "./helpers/BytesWriter";
import {XmlElement} from "xmldoc";
import {H} from "../utility/hash";

const enum TextureDataType {
    Normal = 0,
    CubeMap = 1
}

export class TextureAsset extends Asset {
    static typeName = "texture";

    type = TextureDataType.Normal;
    images: string[] = [];
    webpConfig?: WebpConfig = new WebpConfig();

    constructor(filepath: string) {
        super(filepath, TextureAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
        const type = node.attr.texture_type ?? "2d";
        this.type = TextureDataType.Normal;
        if (type == "2d") {
            this.type = TextureDataType.Normal;
        } else if (type == "cubemap") {
            this.type = TextureDataType.CubeMap;
        } else {
            logger.warn(`Unknown texture type "${type}"`);
        }
        const webp = node.childNamed("webp");
        if (webp) {
            this.webpConfig = new WebpConfig();
            this.webpConfig.lossless = webp.attr.lossless ? (webp.attr.lossless.toLowerCase() == "true") : false;
        }

        this.images = [];
        for (const imageNode of node.childrenNamed("image")) {
            this.images.push(imageNode.attr.path);
        }
    }

    async build() {
        this.readDecl();

        const outputPath = path.join(this.owner.output, this.name);
        makeDirs(path.dirname(outputPath));

        if (this.owner.project.current_target === "ios") {
            this.webpConfig = undefined;
        }

        for (const imagePath of this.images) {
            const srcFilePath = this.getRelativePath(imagePath);
            const destFilepath = path.join(this.owner.output, imagePath);
            copyFile(srcFilePath, destFilepath);
        }
        if (this.webpConfig) {
            const promises = [];
            for (const imagePath of this.images) {
                const filepath = path.join(this.owner.output, imagePath);
                promises.push(compress(filepath, this.webpConfig));
            }
            await Promise.all(promises);
            if (this.owner.project.current_target === "android") {
                for (const imagePath of this.images) {
                    const filepath = path.join(this.owner.output, imagePath);
                    fs.rmSync(filepath);
                }
            }
        }

        const header = new BytesWriter();
        header.writeU32(H(this.typeName));

        // res-name
        header.writeU32(H(this.name));

        // texture data
        header.writeU32(this.type);

        // variants
        let formatMask = 1;
        if (this.webpConfig) {
            if (this.owner.project.current_target === "android") {
                formatMask = 0;
            }
            formatMask |= 2;
        }
        header.writeU32(formatMask);

        header.writeU32(this.images.length);
        for (const image of this.images) {
            header.writeString(image);
        }

        this.owner.writer.writeSection(header);

        return null;
    }
}