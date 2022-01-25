import {Asset} from "./Asset";
import * as path from "path";
import * as glob from "glob";
import {executeAsync, makeDirs} from "../utils";
import {BytesWriter} from "./helpers/BytesWriter";
import {XmlElement} from "xmldoc";
import {H} from "../utility/hash";

export class TranslationsAsset extends Asset {
    static typeName = "translations";

    readonly languages = new Map<string, string>();
    readonly convertedData = new Map<string, ArrayBuffer>();

    constructor(filepath: string) {
        super(filepath, TranslationsAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
        const files = glob.sync(path.join(this.owner.basePath, this.resourcePath, "*.po"));
        for (const file of files) {
            this.languages.set(path.basename(file, ".po"), path.join(this.owner.basePath, this.resourcePath, path.basename(file)));
        }
    }

    async build() {
        this.readDecl();

        const outputPath = path.join(this.owner.output, this.name);
        makeDirs(outputPath);

        const langs: string[] = [];
        for (const lang of this.languages.keys()) {
            langs.push(lang);
            // TODO: `brew install gettext`
            await executeAsync("msgfmt", ["--output-file=" + path.join(outputPath, lang) + ".mo", this.languages.get(lang)]);
        }

        const header = new BytesWriter();
        header.writeU32(H("strings"));
        header.writeString(this.name);
        header.writeU32(langs.length);
        for (let i = 0; i < langs.length; ++i) {
            header.writeFixedASCII(langs[i], 8);
        }
        this.owner.writer.writeSection(header);
    }
}
