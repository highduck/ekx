import {Asset} from "./Asset";
import {makeDirs} from "../utils";
import * as path from "path";
import * as fs from "fs";
import {flashExportAsync} from "./helpers/flashExport";
import {BytesWriter} from "./helpers/BytesWriter";
import {XmlDocument, XmlElement} from "xmldoc";
import {MultiResAtlasAsset} from "./Atlas";
import {H} from "../utility/hash";

export class FlashAsset extends Asset {
    static typeName = "flash";

    targetAtlas: string;

    constructor(filepath: string) {
        super(filepath, FlashAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
        this.targetAtlas = node.attr.atlas;
    }

    async build() {
        makeDirs(path.join(this.owner.cache, this.name));
        const configPath = path.join(this.owner.cache, this.name, "_flash_export_config.xml");
        const sgOutput = path.join(this.owner.output, this.name + ".sg");
        const imagesOutput = path.join(this.owner.cache, this.name, this.targetAtlas);
        const atlasAsset = this.owner.find(MultiResAtlasAsset.typeName, this.targetAtlas) as MultiResAtlasAsset;
        const resolutionNodes = atlasAsset.settings.resolutions.map(r => `<resolution scale="${r.scale}"/>`);
        const xml = new XmlDocument(`<flash path="${path.resolve(this.owner.basePath, this.resourcePath)}" name="${this.name}"
 output="${sgOutput}" outputImages="${imagesOutput}" >
<atlas name="${atlasAsset.settings.name}">
    ${resolutionNodes.join("\n")}
</atlas>
</flash>`);
        fs.writeFileSync(configPath, xml.toString(), "utf-8");
        makeDirs(imagesOutput);
        await flashExportAsync(configPath);
        atlasAsset.inputs.push(path.join(imagesOutput, "_images.xml"));

        // header for .sg file
        const sceneHeader = new BytesWriter();
        sceneHeader.writeU32(H("scene"));
        sceneHeader.writeU32(H(this.name));
        sceneHeader.writeString(this.name + ".sg");
        this.owner.writer.writeSection(sceneHeader);
    }

}