import {Asset, AssetDesc} from "./Asset";
import {isDir, isFile, makeDirs, removePathExtension} from "../utils";
import * as path from "path";
import * as fs from "fs";
import {flashExportAsync} from "./helpers/flashExport";
import {XmlDocument, XmlElement} from "xmldoc";
import {MultiResAtlasAsset} from "./Atlas";
import {H} from "../utility/hash";
import {logger} from "../logger";
import {hashFile, hashGlob} from "./helpers/hash";

export interface FlashDesc extends AssetDesc {
    filepath: string;
    atlas? : string; // main
}

export class FlashAsset extends Asset {
    static typeName = "flash";

    constructor(readonly desc: FlashDesc) {
        super(desc, FlashAsset.typeName);
        desc.name = desc.name ?? removePathExtension(path.basename(desc.filepath));
    }

    resolveInputs(): number {
        const input = path.resolve(this.owner.basePath, this.desc.filepath);
        let hash = super.resolveInputs();
        if(isDir(input)) {
            hash ^= hashGlob(path.join(input, "**/*"));
        }
        else if(isFile(input + ".fla")) {
            hash ^= hashFile(input + ".fla");
        }
        return hash;
    }

    async build() {
        const targetAtlas = this.desc.atlas ?? "main";
        makeDirs(path.join(this.owner.cache, this.desc.name));
        const configPath = path.join(this.owner.cache, this.desc.name, "_flash_export_config.xml");
        const sgOutput = path.join(this.owner.output, this.desc.name + ".sg");
        const imagesOutput = path.join(this.owner.cache, this.desc.name, targetAtlas);
        const atlasAsset = this.owner.find(MultiResAtlasAsset.typeName, targetAtlas) as MultiResAtlasAsset | undefined;
        if(atlasAsset) {
            const resolutionNodes = atlasAsset.desc.resolutions.map(r => `<resolution scale="${r.scale}"/>`);
            const xml = new XmlDocument(`<flash path="${path.resolve(this.owner.basePath, this.desc.filepath)}" name="${this.desc.name}"
 output="${sgOutput}" outputImages="${imagesOutput}" >
<atlas name="${atlasAsset.desc.name}">
    ${resolutionNodes.join("\n")}
</atlas>
</flash>`);
            fs.writeFileSync(configPath, xml.toString(), "utf-8");
            makeDirs(imagesOutput);
            await flashExportAsync(configPath);
            atlasAsset.inputs.push(path.join(imagesOutput, "_images.xml"));

            // header for .sg file
            this.writer.writeU32(H("scene"));
            this.writer.writeU32(H(this.desc.name));
            this.writer.writeString(this.desc.name + ".sg");
        }
        else {
            logger.error(`Atlas target ${targetAtlas} is not found`);
            logger.info("atlases: " + this.owner.map.get(MultiResAtlasAsset.typeName)?.keys());
        }
        return null;
    }

}