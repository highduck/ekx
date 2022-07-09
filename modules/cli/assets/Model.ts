import {Asset, AssetDesc} from "./Asset.ts";
import {path} from "../../deps.ts";
import {makeDirs, removePathExtension} from "../utils.ts";
import {objExportAsync} from "./helpers/objExport.ts";
import {H} from "../utility/hash.ts";
import {hashFile} from "./helpers/hash.ts";

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

        makeDirs(path.dirname(outputPath));

        await objExportAsync(inputPath, outputPath);

        this.writer.writeU32(H(ModelAsset.typeName));
        this.writer.writeString(this.desc.name!);
    }

}