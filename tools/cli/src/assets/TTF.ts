import {Asset, AssetDesc} from "./Asset";
import * as path from "path";
import * as fs from "fs";
import {copyFile, makeDirs, removePathExtension} from "../utils";
import {H} from "../utility/hash";
import {hashFile} from "./helpers/hash";

export interface TTFImporterDesc extends AssetDesc {
    filepath: string;
    glyph_cache?: string;
    base_font_size?: number;
}

export class TTFAsset extends Asset {
    static typeName = "ttf";

    constructor(readonly desc:TTFImporterDesc) {
        super(desc, TTFAsset.typeName);
        desc.name = desc.name ?? removePathExtension(path.basename(desc.filepath));
    }

    resolveInputs(): number {
        return hashFile(path.resolve(this.owner.basePath, this.desc.filepath)) ^
            super.resolveInputs();
    }

    async build() {
        const outputPath = path.join(this.owner.output, this.desc.name + ".ttf");
        makeDirs(path.dirname(outputPath));
        copyFile(path.resolve(this.owner.basePath, this.desc.filepath), outputPath);

        this.writer.writeU32(H(this.typeName));
        this.writer.writeU32(H(this.desc.name));
        this.writer.writeString(this.desc.name + ".ttf");
        this.writer.writeU32(H(this.desc.glyph_cache ?? "default_glyph_cache"));
        this.writer.writeF32(this.desc.base_font_size ?? 48);

        return null;
    }
}