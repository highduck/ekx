import * as path from "path";
import * as fs from "fs";
import {Asset} from "./Asset.js";
import {makeDirs} from "../utils.js";
import {hashFile} from "./helpers/hash.js";

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

    build(): null {
        const inputPath = path.join(this.owner.basePath, this.desc.filepath);
        const outputPath = path.join(this.owner.output, this.desc.filepath);
        makeDirs(path.dirname(outputPath));
        fs.copyFileSync(inputPath, outputPath);
        return null;
    }
}