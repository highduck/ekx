import {Asset, AssetDesc} from "./Asset";
import {makeDirs} from "../utils";
import * as path from "path";
import * as fs from "fs";
import {bmfontAsync} from "./helpers/bmfont";
import {XmlDocument, XmlElement} from "xmldoc";
import {MultiResAtlasAsset} from "./Atlas";
import {H} from "../utility/hash";
import {hashFile} from "./helpers/hash";

export interface BMFontDesc extends AssetDesc {
    filepath: string;
    atlas?: string; // main
    font_size?: number;
    mirror_case?: boolean; // false
    code_range?: {from:number, to:number}[];
    // TODO: change
    font_xml?: string; //
    filters_xml?: string; //
}

export class BitmapFontAsset extends Asset {
    static typeName = "bmfont";

    constructor(readonly desc:BMFontDesc) {
        super(desc, BitmapFontAsset.typeName);
        desc.font_xml = desc.font_xml ?? `<font fontSize="${desc.font_size ?? 24}" mirrorCase="${!!desc.mirror_case}">
        <codeRange from="0x0020" to="0x007F"/>
    </font>`;
        desc.filters_xml = `<filters/>`;
    }

    resolveInputs(): number {
        return hashFile(path.resolve(this.owner.basePath, this.desc.filepath)) ^
            super.resolveInputs();
    }

    async build() {
        const targetAtlas = this.desc.atlas ?? "main";
        makeDirs(path.join(this.owner.cache, this.desc.name));
        const outputFont = path.join(this.owner.output, this.desc.name + ".font");
        const configPath = path.join(this.owner.cache, this.desc.name, "_config.xml");
        const imagesOutput = path.join(this.owner.cache, this.desc.name, targetAtlas);
        const atlasAsset = this.owner.find(MultiResAtlasAsset.typeName, targetAtlas) as MultiResAtlasAsset;
        const xml = new XmlDocument(`<bmfont path="${path.resolve(this.owner.basePath, this.desc.filepath)}" name="${this.desc.name}"
 outputSprites="${imagesOutput}"
  outputFont="${outputFont}">
${this.desc.filters_xml!}
${this.desc.font_xml!}
<atlas name="${atlasAsset.desc.name}">
    ${atlasAsset.desc.resolutions.map(r => "<resolution scale=\"" + r.scale + "\"/>").join("\n")}
</atlas>
</bmfont>`);
        fs.writeFileSync(configPath, xml.toString(), "utf-8");

        // prepare required folder for images collection
        makeDirs(imagesOutput);
        await bmfontAsync(configPath);

        atlasAsset.inputs.push(path.join(imagesOutput, "_images.xml"));

        this.writer.writeU32(H(BitmapFontAsset.typeName));
        this.writer.writeU32(H(this.desc.name));
        this.writer.writeString(this.desc.name + ".font");
    }
}
