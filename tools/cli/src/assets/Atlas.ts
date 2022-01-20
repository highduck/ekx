import {compress, WebpConfig} from "./helpers/webp";
import {readFloat} from "./helpers/parse";
import {Asset} from "./Asset";
import {makeDirs} from "../utils";
import * as path from "path";
import * as fs from "fs";
import * as glob from "glob";
import {spritePackerAsync} from "./helpers/spritePacker";
import {BytesWriter} from "./helpers/BytesWriter";
import {XmlElement} from "xmldoc";
import {H} from "../utility/hash";

class MRAResolution {
    scale = 1.0;
    maxWidth = 2048;
    maxHeight = 2048;
}

export class MultiResAtlasSettings {
    name: string;
    resolutions: MRAResolution[] = [];
    webpConfig?: WebpConfig = new WebpConfig();

    readFromXML(node: XmlElement) {
        this.name = node.attr.name;
        for (const resolutionNode of node.childrenNamed("resolution")) {
            const res = new MRAResolution();
            res.scale = readFloat(resolutionNode.attr.scale, res.scale);
            res.maxWidth = readFloat(resolutionNode.attr.max_width, res.maxWidth);
            res.maxHeight = readFloat(resolutionNode.attr.max_height, res.maxHeight);
            this.resolutions.push(res);
        }
        const webp = node.childNamed("webp");
        if (webp) {
            this.webpConfig = new WebpConfig();
            this.webpConfig.lossless = webp.attr.lossless ? (webp.attr.lossless.toLowerCase() == "true") : true;
        }
    }
}

export class MultiResAtlasAsset extends Asset {
    static typeName = "atlas_builder";

    readonly settings = new MultiResAtlasSettings();
    xmlSource: string;
    inputs: string[] = [];

    constructor(filepath: string) {
        super(filepath, MultiResAtlasAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
        this.settings.readFromXML(node);
        this.xmlSource = node.toString();
    }

    async afterBuild() {
        makeDirs(path.join(this.owner.cache, this.name));
        const configPath = path.join(this.owner.cache, this.name, "_config.xml");

        const resolutionNodes = [];
        for (const r of this.settings.resolutions) {
            resolutionNodes.push(`<resolution scale="${r.scale}" maxWidth="${r.maxWidth}" maxHeight="${r.maxHeight}"/>`);
        }
        const inputNodes = [];
        for (const i of this.inputs) {
            inputNodes.push(`<images path="${i}"/>`);
        }
        const xml = `<atlas path="${path.resolve(this.owner.basePath, this.resourcePath)}" name="${this.name}" output="${this.owner.output}">
    ${resolutionNodes.join("\n")}
    ${inputNodes.join("\n")}
</atlas>`;
        fs.writeFileSync(configPath, xml, "utf8");
        await spritePackerAsync(configPath);

        if (this.owner.project.current_target === "ios") {
            this.settings.webpConfig = undefined;
        }

        if (this.settings.webpConfig) {
            const files = glob.sync(path.join(this.owner.output, `**/${this.name}*.png`));
            const promises = [];
            for (const filepath of files) {
                promises.push(compress(filepath, this.settings.webpConfig));
            }
            await Promise.all(promises);

            if (this.owner.project.current_target === "android") {
                for (const filepath of files) {
                    fs.rmSync(filepath);
                }
            }
        }

        const header = new BytesWriter();
        header.writeU32(H("atlas"));
        header.writeString(this.name);
        // variants
        let formatMask = 1;
        if (this.settings.webpConfig) {
            if (this.owner.project.current_target === "android") {
                formatMask = 0;
            }
            formatMask |= 2;
        }
        header.writeU32(formatMask);
        this.owner.writer.writeSection(header);
    }
}