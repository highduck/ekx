import * as path from "path";
import {Asset, AssetDesc} from "./Asset.js";
import {removePathExtension} from "../utils.js";
import {objExportAsync} from "./helpers/objExport.js";
import {H} from "../utility/hash.js";
import {hashFile} from "./helpers/hash.js";
import {ensureDirSync} from "../../utils/utils.js";

export interface ObjImporterDesc extends AssetDesc {
    filepath: string;
}

export class ModelAsset extends Asset {
    static typeName = "model";

    constructor(readonly desc: ObjImporterDesc) {
        super(desc, ModelAsset.typeName);
        this.desc.name = this.desc.name ?? removePathExtension(path.basename(desc.filepath));
    }

    resolveInputs(): number {
        return super.resolveInputs() ^ hashFile(path.resolve(this.owner.basePath, this.desc.filepath));
    }

    async build() {
        // TODO: change ext to `m3d`
        const outputPath = path.join(this.owner.output, this.desc.name + ".model");
        const inputPath = path.join(this.owner.basePath, this.desc.filepath);

        ensureDirSync(path.dirname(outputPath));

        await objExportAsync(inputPath, outputPath);

        this.writer.writeU32(H(ModelAsset.typeName));
        this.writer.writeString(this.desc.name!);
    }

}