import {Asset} from "./Asset";
import * as path from "path";
import {copyFile, makeDirs, removePathExtension} from "../utils";
import {BytesWriter} from "./helpers/BytesWriter";
import {H} from "../utility/hash";
import {hashFile} from "./helpers/hash";

export class StaticFileDescription {
    filepath: string;
    dev?: boolean; // false
}

export class StaticFileImporter extends Asset {
    static typeName = "static";

    constructor(readonly desc: StaticFileDescription) {
        super(desc, StaticFileImporter.typeName);
    }

    resolveInputs(): number {
        return super.resolveInputs() ^ hashFile(path.resolve(this.owner.basePath, this.desc.filepath));
    }

    async build() {
        const inputPath = path.join(this.owner.basePath, this.desc.filepath);
        const outputPath = path.join(this.owner.output, this.desc.filepath);
        makeDirs(path.dirname(outputPath));
        copyFile(inputPath, outputPath);
        return null;
    }
}