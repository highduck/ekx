import {Asset} from "./Asset";
import * as path from "path";
import {makeDirs} from "../utils";
import {objExportAsync} from "./helpers/objExport";
import {XmlElement} from "xmldoc";

export class ModelAsset extends Asset {
    static typeName = "model";

    constructor(filepath: string) {
        super(filepath, ModelAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
    }

    async build() {
        const outputPath = path.join(this.owner.output, this.name + ".model");
        const inputPath = this.getRelativePath(this.resourcePath);

        makeDirs(path.dirname(outputPath));

        await objExportAsync(inputPath, outputPath);

        this.owner.meta(this.typeName, this.name);
    }

}