import * as path from "path";
import {XmlDocument} from "xmldoc";
import {Asset, AssetDesc} from "./Asset.js";
import {bmfont} from "./helpers/bmfont.js";
import {MultiResAtlasAsset} from "./Atlas.js";
import {H} from "../utility/hash.js";
import {hashFile} from "./helpers/hash.js";
import {ensureDirSync, writeTextFileSync} from "../../utils/utils.js";

export interface BMFontDesc extends AssetDesc {
    filepath: string;
    atlas?: string; // main
    font_size?: number;
    mirror_case?: boolean; // false
    use_kerning?: boolean; // true
    code_range?: { from: number, to: number }[];
}

export class BitmapFontAsset extends Asset {
    static typeName = "bmfont";

    constructor(readonly desc: BMFontDesc) {
        super(desc, BitmapFontAsset.typeName);
    //     desc.font_xml = desc.font_xml ?? `<font fontSize="${desc.font_size ?? 24}" mirrorCase="${!!desc.mirror_case}">
    //     <codeRange from="0x0020" to="0x007F"/>
    // </font>`;
    //     desc.filters_xml = `<filters/>`;
    }

    resolveInputs(): number {
        return hashFile(path.resolve(this.owner.basePath, this.desc.filepath)) ^
            super.resolveInputs();
    }

    async build() {
        const targetAtlas = this.desc.atlas ?? "main";
        ensureDirSync(path.join(this.owner.cache, this.desc.name!));
        const outputFont = path.join(this.owner.output, this.desc.name + ".font");
        const configPath = path.join(this.owner.cache, this.desc.name!, "config.txt");
        const imagesOutput = path.join(this.owner.cache, this.desc.name!, targetAtlas);
        const atlasAsset = this.owner.find(MultiResAtlasAsset.typeName, targetAtlas) as MultiResAtlasAsset;
        const codeRanges = this.desc.code_range ?? [{from: 0x0020, to: 0x007F}];
        const resolutions = atlasAsset.desc.resolutions ?? [{scale: 1}, {scale: 2}, {scale: 3}, {scale: 4}];
        //output_sprites, output_font, font_name, ttf_path
        let config = `${imagesOutput}
${outputFont}
${this.desc.name}
${path.resolve(this.owner.basePath, this.desc.filepath)}
${this.desc.font_size ?? 24}
${codeRanges.length}
${atlasAsset.desc.resolutions?.length ?? 4}
0
${(this.desc.mirror_case ?? false) ? 1 : 0}
${(this.desc.use_kerning ?? true) ? 1 : 0}
${atlasAsset.desc.name}
${codeRanges.map(cr => cr.from + " " + cr.to).join("\n")}
${resolutions.map(r=> ""+r.scale).join("\n")}
`;
//         from="0x0020" to="0x007F"
//         const xml = new XmlDocument(`<bmfont path="${path.resolve(this.owner.basePath, this.desc.filepath)}" name="${this.desc.name}"
//  outputSprites="${imagesOutput}"
//   outputFont="${outputFont}">
// ${this.desc.filters_xml!}
// ${this.desc.font_xml!}
// <atlas name="${atlasAsset.desc.name}">
//     ${atlasAsset.desc.resolutions!.map(r => "<resolution scale=\"" + r.scale + "\"/>").join("\n")}
// </atlas>
// </bmfont>`);
        writeTextFileSync(configPath, config);

        // prepare required folder for images collection
        ensureDirSync(imagesOutput);
        await bmfont(configPath);

        atlasAsset.inputs.push(path.join(imagesOutput, "images.txt"));

        this.writer.writeU32(H(BitmapFontAsset.typeName));
        this.writer.writeU32(H(this.desc.name!));
        this.writer.writeString(this.desc.name + ".font");
    }
}
