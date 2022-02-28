import {Asset, AssetDesc} from "./Asset";
import * as path from "path";
import * as glob from "glob";
import {executeAsync, makeDirs} from "../utils";
import {XmlElement} from "xmldoc";
import {H} from "../utility/hash";
import {hashFile, hashGlob} from "./helpers/hash";

export interface TranslationsDesc extends AssetDesc {
    filepath: string
}

export class TranslationsAsset extends Asset {
    static typeName = "translations";

    readonly languages = new Map<string, string>();
    readonly convertedData = new Map<string, ArrayBuffer>();

    constructor(readonly desc:TranslationsDesc) {
        super(desc, TranslationsAsset.typeName);
        desc.name = desc.name ?? path.basename(desc.filepath);
    }

    resolveInputs(): number {
        return super.resolveInputs() ^ hashGlob(path.join(this.owner.basePath, this.desc.filepath, "*.po"));
    }

    async build() {
        const files = glob.sync(path.join(this.owner.basePath, this.desc.filepath, "*.po"));
        for (const file of files) {
            this.languages.set(path.basename(file, ".po"), path.join(this.owner.basePath, this.desc.filepath, path.basename(file)));
        }

        const outputPath = path.join(this.owner.output, this.desc.name);
        makeDirs(outputPath);

        const langs: string[] = [];
        for (const lang of this.languages.keys()) {
            langs.push(lang);
            // TODO: `brew install gettext`
            await executeAsync("msgfmt", ["--output-file=" + path.join(outputPath, lang) + ".mo", this.languages.get(lang)]);
        }

        this.writer.writeU32(H("strings"));
        this.writer.writeString(this.desc.name);
        this.writer.writeU32(langs.length);
        for (let i = 0; i < langs.length; ++i) {
            this.writer.writeFixedASCII(langs[i], 8);
        }
    }
}
