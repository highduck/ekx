import {Asset} from "./Asset";
import * as path from "path";
import {copyFile, makeDirs} from "../utils";
import {BytesWriter} from "./helpers/BytesWriter";
import * as glob from "glob";
import {XmlElement} from "xmldoc";
import {H} from "../utility/hash";

class AudioFile {
    filepath: string;
    streaming: boolean;
}

export class AudioAsset extends Asset {
    static typeName = "audio";

    list: AudioFile[] = [];

    constructor(filepath: string) {
        super(filepath, AudioAsset.typeName);
    }

    checkFilters(filepath: string, filters: string[]) {
        for (const filter of filters) {
            if (filepath.indexOf(filter) >= 0) {
                return true;
            }
        }
        return false;
    }

    readDeclFromXml(node: XmlElement) {
        const musicFilters: string[] = [];

        for (const musicNode of node.childrenNamed("music")) {
            musicFilters.push(musicNode.attr.filter);
        }

        const files = glob.sync(path.join(this.owner.basePath, this.resourcePath, "*.mp3"));
        for (const file of files) {
            this.list.push({
                filepath: file,
                streaming: this.checkFilters(file, musicFilters)
            });
        }
    }

    async build() {
        // `name`\ folder for all sounds
        const outputPath = path.join(this.owner.output, this.name);
        makeDirs(outputPath);

        for (const audio of this.list) {
            const p = path.join(this.name, path.basename(audio.filepath));
            copyFile(audio.filepath, path.join(this.owner.output, p));
            const header = new BytesWriter();
            header.writeU32(H("audio"));
            // remove extension for resource name
            header.writeU32(H(p.substring(0, p.length - 4)));
            header.writeU32(audio.streaming ? 1 : 0);
            header.writeString(p);
            this.owner.writer.writeSection(header);
        }
        return null;
    }
}