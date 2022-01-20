import {Asset} from "./Asset";
import {makeDirs} from "../utils";
import * as path from "path";
import * as fs from "fs";
import {bmfontAsync} from "./helpers/bmfont";
import {BytesWriter} from "./helpers/BytesWriter";
import {XmlDocument, XmlElement} from "xmldoc";
import {MultiResAtlasAsset} from "./Atlas";
import {H} from "../utility/hash";

export class BitmapFontAsset extends Asset {
    static typeName = "bmfont";

    targetAtlas: string;
    filters = "";
    font = "";

    constructor(filepath: string) {
        super(filepath, BitmapFontAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
        this.targetAtlas = node.attr.atlas ?? "main";
        this.filters = node.childNamed("filters").toString();
        this.font = node.childNamed("font").toString();
    }

    async build() {
        makeDirs(path.join(this.owner.cache, this.name));
        const outputFont = path.join(this.owner.output, this.name + ".font");
        const configPath = path.join(this.owner.cache, this.name, "_config.xml");
        const imagesOutput = path.join(this.owner.cache, this.name, this.targetAtlas);
        const atlasAsset = this.owner.find(MultiResAtlasAsset.typeName, this.targetAtlas) as MultiResAtlasAsset;
        const xml = new XmlDocument(`<bmfont path="${path.resolve(this.owner.basePath, this.resourcePath)}" name="${this.name}"
 outputSprites="${imagesOutput}"
  outputFont="${outputFont}">
${this.filters}
${this.font}
<atlas name="${atlasAsset.settings.name}">
    ${atlasAsset.settings.resolutions.map(r => "<resolution scale=\"" + r.scale + "\"/>").join("\n")}
</atlas>
</bmfont>`);
        fs.writeFileSync(configPath, xml.toString(), "utf-8");

        // prepare required folder for images collection
        makeDirs(imagesOutput);
        await bmfontAsync(configPath);

        atlasAsset.inputs.push(path.join(imagesOutput, "_images.xml"));

        const header = new BytesWriter();
        header.writeU32(H(BitmapFontAsset.typeName));
        header.writeU32(H(this.name));
        header.writeString(this.name + ".font");
        this.owner.writer.writeSection(header);
    }
}
