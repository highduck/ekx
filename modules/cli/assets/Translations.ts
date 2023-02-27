import * as path from "path";
import {Asset, AssetDesc} from "./Asset.js";
import {H} from "../utility/hash.js";
import {hashGlob} from "./helpers/hash.js";
import {ensureDirSync, expandGlobSync} from "../../utils/utils.js";
import {msgfmt} from "./helpers/msgfmt.js";

export interface TranslationsDesc extends AssetDesc {
    filepath: string
}

export class TranslationsAsset extends Asset {
    static typeName = "translations";

    readonly languages = new Map<string, string>();
    readonly convertedData = new Map<string, ArrayBuffer>();

    constructor(readonly desc: TranslationsDesc) {
        super(desc, TranslationsAsset.typeName);
        desc.name = desc.name ?? path.basename(desc.filepath);
    }

    resolveInputs(): number {
        return super.resolveInputs() ^ hashGlob(path.join(this.owner.basePath, this.desc.filepath, "*.po"));
    }

    async build() {
        const files = expandGlobSync(path.join(this.owner.basePath, this.desc.filepath, "*.po"));
        for (const file of files) {
            this.languages.set(path.basename(file.path, ".po"), path.join(this.owner.basePath, this.desc.filepath, path.basename(file.path)));
        }

        const outputPath = path.join(this.owner.output, this.desc.name!);
        ensureDirSync(outputPath);

        const langs: string[] = [];
        for (const [lang, filepath] of this.languages) {
            langs.push(lang);
            await msgfmt(filepath, path.join(outputPath, lang) + ".mo");
        }

        this.writer.writeU32(H("strings"));
        this.writer.writeString(this.desc.name!);
        this.writer.writeFixedASCIIArray(langs, 8);
    }
}
