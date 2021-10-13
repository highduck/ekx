import {Asset} from "./ap";
import {XmlDocument, XmlElement} from "xmldoc";
import * as path from "path";
import {copyFile, executeAsync, makeDirs} from "../utils";
import {BytesWriter} from "./BytesWriter";
import * as glob from "glob";
import {logger} from "../logger";
import {parseBoolean, readFloat} from "./parse";
import * as fs from "fs";
import {bmfontAsync} from "./bmfont";
import {spritePackerAsync} from "./spritePacker";
import {flashExportAsync} from "./flashExport";
import {objExportAsync} from "./objExport";
import {compress, WebpConfig} from "./webp";

export class TTFAsset extends Asset {
    static typeName = "ttf";

    glyphCache: string;
    baseFontSize: number;

    constructor(filepath: string) {
        super(filepath, TTFAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
        this.glyphCache = node.attr.glyphCache ?? "default_glyph_cache";
        this.baseFontSize = node.attr.fontSize ? parseFloat(node.attr.fontSize) : 48;
    }

    async build() {
        this.readDecl();

        const outputPath = path.join(this.owner.output, this.name);
        makeDirs(path.dirname(outputPath));
        copyFile(path.resolve(this.owner.basePath, this.resourcePath), outputPath + ".ttf");

        const header = new BytesWriter(32);
        header.writeString(this.typeName);
        header.writeString(this.name);
        header.writeString(this.name + ".ttf");
        header.writeString(this.glyphCache);
        header.writeF32(this.baseFontSize);

        this.owner.writer.writeSection(header);

        return null;
    }
}

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
        header.writeString("strings");
        header.writeString(this.name);
        header.writeU32(langs.length);
        for (let i = 0; i < langs.length; ++i) {
            header.writeString(langs[i]);
        }
        this.owner.writer.writeSection(header);
    }
}

const enum TextureDataType {
    Normal = 0,
    CubeMap = 1
}

// const WEBP_OPTIONS = ["-near_lossless", "60"];
const WEBP_OPTIONS = ["-preset", "icon"];

export class TextureAsset extends Asset {
    static typeName = "texture";

    type = TextureDataType.Normal;
    images: string[] = [];
    webpConfig?: WebpConfig;

    constructor(filepath: string) {
        super(filepath, TextureAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
        const type = node.attr.texture_type ?? "2d";
        this.type = TextureDataType.Normal;
        if (type == "2d") {
            this.type = TextureDataType.Normal;
        } else if (type == "cubemap") {
            this.type = TextureDataType.CubeMap;
        } else {
            logger.warn(`Unknown texture type "${type}"`);
        }
        const webp = node.childNamed("webp");
        if (webp) {
            this.webpConfig = new WebpConfig();
            this.webpConfig.lossless = webp.attr.lossless ? (webp.attr.lossless.toLowerCase() == "true") : false;
        }

        this.images = [];
        for (const imageNode of node.childrenNamed("image")) {
            this.images.push(imageNode.attr.path);
        }
    }

    async build() {
        this.readDecl();

        const outputPath = path.join(this.owner.output, this.name);
        makeDirs(path.dirname(outputPath));

        if (this.owner.project.current_target === "ios") {
            this.webpConfig = undefined;
        }

        for (const imagePath of this.images) {
            const srcFilePath = this.getRelativePath(imagePath);
            const destFilepath = path.join(this.owner.output, imagePath);
            copyFile(srcFilePath, destFilepath);
        }
        if (this.webpConfig) {
            const promises = [];
            for (const imagePath of this.images) {
                const filepath = path.join(this.owner.output, imagePath);
                promises.push(compress(filepath, this.webpConfig));
            }
            await Promise.all(promises);
            if (this.owner.project.current_target === "android") {
                for (const imagePath of this.images) {
                    const filepath = path.join(this.owner.output, imagePath);
                    fs.rmSync(filepath);
                }
            }
        }

        const header = new BytesWriter();
        header.writeString(this.typeName);

        // res-name
        header.writeString(this.name);

        // texture data
        header.writeU32(this.type);

        // variants
        let formatMask = 1;
        if (this.webpConfig) {
            if (this.owner.project.current_target === "android") {
                formatMask = 0;
            }
            formatMask |= 2;
        }
        header.writeU32(formatMask);

        header.writeU32(this.images.length);
        for (const image of this.images) {
            header.writeString(image);
        }

        this.owner.writer.writeSection(header);

        return null;
    }
}

class MRAResolution {
    scale = 1.0;
    maxWidth = 2048;
    maxHeight = 2048;
}

export class MultiResAtlasSettings {
    name: string;
    resolutions: MRAResolution[] = [];
    webpConfig?: WebpConfig;

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
        header.writeString("atlas");
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

export class ModelAsset extends Asset {
    static typeName = "model";

    constructor(filepath: string) {
        super(filepath, ModelAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
    }

    async build() {
        const outputPath = path.join(this.owner.output, this.name + ".model");
        const inputPath = this.getRelativePath(this.resourcePath);

        makeDirs(path.dirname(outputPath));

        await objExportAsync(inputPath, outputPath);

        this.owner.meta(this.typeName, this.name);
    }

}

export class FlashAsset extends Asset {
    static typeName = "flash";

    targetAtlas: string;

    constructor(filepath: string) {
        super(filepath, FlashAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
        this.targetAtlas = node.attr.atlas;
    }

    async build() {
        makeDirs(path.join(this.owner.cache, this.name));
        const configPath = path.join(this.owner.cache, this.name, "_config.xml");
        const sgOutput = path.join(this.owner.output, this.name + ".sg");
        const imagesOutput = path.join(this.owner.cache, this.name, this.targetAtlas);
        const atlasAsset = this.owner.find(MultiResAtlasAsset.typeName, this.targetAtlas) as MultiResAtlasAsset;
        const resolutionNodes = atlasAsset.settings.resolutions.map(r => `<resolution scale="${r.scale}"/>`);
        const xml = new XmlDocument(`<flash path="${path.resolve(this.owner.basePath, this.resourcePath)}" name="${this.name}"
 output="${sgOutput}" outputImages="${imagesOutput}" >
<atlas name="${atlasAsset.settings.name}">
    ${resolutionNodes.join("\n")}
</atlas>
</flash>`);
        fs.writeFileSync(configPath, xml.toString(), "utf-8");
        makeDirs(imagesOutput);
        await flashExportAsync(configPath);
        atlasAsset.inputs.push(path.join(imagesOutput, "_images.xml"));

        // header for .sg file
        const sceneHeader = new BytesWriter();
        sceneHeader.writeString("scene");
        sceneHeader.writeString(this.name);
        sceneHeader.writeString(this.name + ".sg");
        this.owner.writer.writeSection(sceneHeader);
    }

}

export class DynamicAtlasAsset extends Asset {
    static typeName = "dynamic_atlas";

    alphaMap = false;
    mipmaps = false;

    constructor(filepath: string) {
        super(filepath, DynamicAtlasAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
        this.alphaMap = parseBoolean(node.attr.alphaMap);
        this.mipmaps = parseBoolean(node.attr.mipmaps);
    }

    async build() {
        let flags = 0;
        if (this.alphaMap) flags |= 1;
        if (this.mipmaps) flags |= 2;

        const writer = new BytesWriter(8);
        writer.writeString(DynamicAtlasAsset.typeName);
        writer.writeString(this.name);
        writer.writeU32(flags);

        this.owner.writer.writeSection(writer);

        return null;
    }

}

export class BitmapFontAsset extends Asset {
    static typeName = "bmfont";

    targetAtlas: string;
    filters = "";
    font = "";

    constructor(filepath: string) {
        super(filepath, BitmapFontAsset.typeName);
    }

    readDeclFromXml(node: XmlElement) {
        this.targetAtlas = node.attr.atlas ?? "main";
        this.filters = node.childNamed("filters").toString();
        this.font = node.childNamed("font").toString();
    }

    async build() {
        makeDirs(path.join(this.owner.cache, this.name));
        const outputFont = path.join(this.owner.output, this.name + ".font");
        const configPath = path.join(this.owner.cache, this.name, "_config.xml");
        const imagesOutput = path.join(this.owner.cache, this.name, this.targetAtlas);
        const atlasAsset = this.owner.find(MultiResAtlasAsset.typeName, this.targetAtlas) as MultiResAtlasAsset;
        const xml = new XmlDocument(`<bmfont path="${path.resolve(this.owner.basePath, this.resourcePath)}" name="${this.name}"
 outputSprites="${imagesOutput}"
  outputFont="${outputFont}">
${this.filters}
${this.font}
<atlas name="${atlasAsset.settings.name}">
    ${atlasAsset.settings.resolutions.map(r => "<resolution scale=\"" + r.scale + "\"/>").join("\n")}
</atlas>
</bmfont>`);
        fs.writeFileSync(configPath, xml.toString(), "utf-8");

        // prepare required folder for images collection
        makeDirs(imagesOutput);
        await bmfontAsync(configPath);

        atlasAsset.inputs.push(path.join(imagesOutput, "_images.xml"));

        const header = new BytesWriter();
        header.writeString(BitmapFontAsset.typeName);
        header.writeString(this.name);
        header.writeString(this.name + ".font");
        this.owner.writer.writeSection(header);
    }
}

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

    checkFilters(path: string, filters: string[]) {
        for (const filter of filters) {
            if (path.indexOf(filter) >= 0) {
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
            header.writeString("audio");
            // remove extension for resource name
            header.writeString(p.substring(0, p.length - 4));
            header.writeString(p);
            header.writeU32(audio.streaming ? 1 : 0);
            this.owner.writer.writeSection(header);
        }
        return null;
    }
}