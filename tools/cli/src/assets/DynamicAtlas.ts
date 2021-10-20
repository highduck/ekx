import {Asset} from "./Asset";
import {parseBoolean} from "./helpers/parse";
import {BytesWriter} from "./helpers/BytesWriter";
import {XmlElement} from "xmldoc";

export class DynamicAtlasAsset extends Asset {
    static typeName = "dynamic_atlas";

    alphaMap = false;
    mipmaps = false;

    constructor(filepath: string) {
        super(filepath, DynamicAtlasAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
        this.alphaMap = parseBoolean(node.attr.alphaMap);
        this.mipmaps = parseBoolean(node.attr.mipmaps);
    }

    async build() {
        let flags = 0;
        if (this.alphaMap) flags |= 1;
        if (this.mipmaps) flags |= 2;

        const writer = new BytesWriter(8);
        writer.writeString(DynamicAtlasAsset.typeName);
        writer.writeString(this.name);
        writer.writeU32(flags);

        this.owner.writer.writeSection(writer);

        return null;
    }

}