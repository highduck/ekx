import {Asset} from "./Asset.ts";
import {path} from "../../deps.ts";
import {copyFile, makeDirs} from "../utils.ts";
import {hashFile} from "./helpers/hash.ts";

export class StaticFileDescription {
    filepath = "";
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

    build() {
        const inputPath = path.join(this.owner.basePath, this.desc.filepath);
        const outputPath = path.join(this.owner.output, this.desc.filepath);
        makeDirs(path.dirname(outputPath));
        copyFile(inputPath, outputPath);
        return null;
    }
}