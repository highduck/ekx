import * as path from "path";
import * as fs from "fs";
import {Asset} from "./Asset.js";
import {removePathExtension} from "../utils.js";
import {H} from "../utility/hash.js";
import {hashFile} from "./helpers/hash.js";
import {ensureDirSync} from "../../utils/utils.js";

export interface AudioFile {
    filepath: string;
    name?: string; // filepath without extension
    lazy?: boolean; // true
    dev?: boolean; // false

    // audio specific
    streaming?: boolean; // false
}

export class AudioAsset extends Asset {
    static typeName = "audio";

    constructor(readonly desc: AudioFile) {
        super(desc, AudioAsset.typeName);
        const streaming = this.desc.streaming ?? false;
        const lazy = this.desc.lazy ?? true;
        this.priority = 100;
        if (streaming) {
            this.priority += 1;
        }
        if (lazy) {
            this.priority += 1;
        }
    }

    resolveInputs(): number {
        return hashFile(path.resolve(this.owner.basePath, this.desc.filepath)) ^
            super.resolveInputs();
    }

    build(): null {
        const outputPath = path.join(this.owner.output, this.desc.filepath);
        const inputPath = path.join(this.owner.basePath, this.desc.filepath);
        ensureDirSync(path.dirname(outputPath));

        fs.copyFileSync(inputPath, outputPath);
        const name = this.desc.name ?? removePathExtension(this.desc.filepath);
        const streaming = this.desc.streaming ?? false;
        const lazy = this.desc.lazy ?? true;
        const dev = this.desc.dev ?? false;
        if (dev && !this.owner.devMode) {
            return null;
        }
        this.writer.writeU32(H("audio"));
        this.writer.writeU32(H(name));
        let flags = 0;
        if (streaming) {
            flags |= 1;
        }
        if (lazy) {
            flags |= 2;
        }
        this.writer.writeU32(flags);
        this.writer.writeString(this.desc.filepath);

        return null;
    }
}