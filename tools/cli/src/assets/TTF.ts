import {Asset} from "./Asset";
import * as path from "path";
import {copyFile, makeDirs} from "../utils";
import {BytesWriter} from "./helpers/BytesWriter";
import {XmlElement} from "xmldoc";

export class TTFAsset extends Asset {
    static typeName = "ttf";

    glyphCache: string;
    baseFontSize: number;

    constructor(filepath: string) {
        super(filepath, TTFAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
        this.glyphCache = node.attr.glyphCache ?? "default_glyph_cache";
        this.baseFontSize = node.attr.fontSize ? parseFloat(node.attr.fontSize) : 48;
    }

    async build() {
        this.readDecl();

        const outputPath = path.join(this.owner.output, this.name);
        makeDirs(path.dirname(outputPath));
        copyFile(path.resolve(this.owner.basePath, this.resourcePath), outputPath + ".ttf");

        const header = new BytesWriter(32);
        header.writeString(this.typeName);
        header.writeString(this.name);
        header.writeString(this.name + ".ttf");
        header.writeString(this.glyphCache);
        header.writeF32(this.baseFontSize);

        this.owner.writer.writeSection(header);

        return null;
    }
}