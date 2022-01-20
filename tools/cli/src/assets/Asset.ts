import {makeDirs, readText} from "../utils";
import {logger} from "../logger";
import * as path from "path";
import {parseBoolean} from "./helpers/parse";
import {XmlDocument, XmlElement} from "xmldoc";
import {BytesWriter} from "./helpers/BytesWriter";
import {Project} from "../project";
import * as fs from "fs";
import * as glob from "glob";
import {H} from "../utility/hash";

interface AssetInfoHeader {
    type: string;
    dev?: boolean;
}

function openAssetInfoHeader(filepath: string): AssetInfoHeader | null {
    try {
        const text = readText(filepath);
        const doc = new XmlDocument(text);
        return {
            type: doc.attr.type,
            dev: !!doc.attr.dev
        };
    } catch (err: any) {
        logger.error(`XML parsing error "${filepath}"`, err);
    }
    return null;
}

function getDefaultName(filepath: string): string {
    return path.basename(filepath, ".asset.xml");
}

export class Asset {
    owner!: AssetBuilder;
    resourcePath: string;
    dev: boolean = false;
    name: string;

    constructor(readonly declarationPath: string,
                readonly typeName: string) {

    }

    readDecl() {
        const fullPath = this.declarationPath;
        const text = readText(fullPath);
        const xml = new XmlDocument(text);
        if (xml) {
            const defaultName = getDefaultName(this.declarationPath);
            this.name = xml.attr.name ?? defaultName;
            this.resourcePath = xml.attr.path ?? defaultName;
            this.dev = parseBoolean(xml.attr.dev);
            this.readDeclFromXml(xml);
        } else {
            logger.error(`error parse xml "${fullPath}"`);
        }
    }

    readDeclFromXml(xml: XmlElement) {

    }

    beforeBuild(): null | Promise<void> {
        return null;
    }

    build(): null | Promise<void> {
        return null;
    }

    afterBuild(): null | Promise<void> {
        return null;
    }

    // get path near declaration (same folder)
    getRelativePath(filepath: string): string {
        return path.join(path.dirname(this.declarationPath), filepath);
    }
}


export class AssetBuilder {
    readonly writer = new BytesWriter();

    meta(typeName: string, filepath: string) {
        const header = new BytesWriter();
        header.writeU32(H(typeName));
        header.writeString(filepath);
        this.writer.writeSection(header);
    }

    cache: string;
    output: string;
    devMode = true;
    assets: Asset[] = [];
    map = new Map<string, Map<string, Asset>>();

    readonly factory = new Map<string, (path: string) => Asset>();

    constructor(readonly project: Project, public basePath: string = "../assets") {

    }

    register(asset: Asset) {
        const type = asset.typeName;
        let m = this.map.get(type);
        if (!m) {
            m = new Map();
            this.map.set(type, m);
        }
        m.set(asset.name, asset);
    }

    find(type: string, name: string): Asset | undefined {
        const m = this.map.get(type);
        if (m) {
            return m.get(name);
        }
        return undefined;
    }

    async runPhase(assets: Asset[], phase: "afterBuild" | "build" | "beforeBuild") {
        const bb = [];
        for (const asset of assets) {
            const task = asset[phase]();
            if (task !== null) {
                bb.push(task);
            }
        }
        if (bb.length > 0) {
            await Promise.all(bb);
        }
    }

    async build(output: string, cache: string) {
        this.output = output;
        this.cache = cache;
        const assetsToBuild: Asset[] = [];
        // read declaration for all populated assets,
        // and add to queue assets that should be packed (dev / platform)
        // platform filter is not implemented yet
        for (const asset of this.assets) {
            asset.readDecl();
            // skip dev files for build without editor
            if (this.devMode || !asset.dev) {
                assetsToBuild.push(asset);
            }
            this.register(asset);
        }

        makeDirs(output);

        // then all queued assets should pass 3 steps: before build, build, after build
        logger.info("Build Before");
        await this.runPhase(assetsToBuild, "beforeBuild");
        logger.info("Build Main");
        await this.runPhase(assetsToBuild, "build");
        logger.info("Build After");
        await this.runPhase(assetsToBuild, "afterBuild");

        this.writer.writeU32(0);
        this.writer.save(path.join(output, "pack.bin"));
    }

    add(filepath: string) {
        const header = openAssetInfoHeader(filepath);
        if (!header) {
            return;
        }
        const factoryMethod = this.factory.get(header.type);
        if (!factoryMethod) {
            logger.error(`Editor asset type "${header.type}" not found`);
            return;
        }
        const asset = factoryMethod(filepath);
        asset.owner = this;
        this.assets.push(asset);
    }

    populate() {
        try {
            const bp = fs.realpathSync(this.basePath);
            const files = glob.sync(path.join(bp, "**/*.asset.xml"));
            for (const file of files) {
                this.add(file);
            }
        }
        catch(err:any) {
            logger.warn(`Cannot search asset files in ${this.basePath} path ⬇ \n`, err);
        }
    }

}