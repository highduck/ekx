import * as fs from "fs";
import * as path from "path";
import {isDir, isFile} from "../utils.js";
import {logger} from "../logger.js";
import {BytesWriter} from "./helpers/BytesWriter.js";
import {Project} from "../project.js";
import {H} from "../utility/hash.js";
import {ensureDirSync, expandGlobSync, readTextFileSync, writeTextFileSync} from "../../utils/utils.js";

export interface AssetDesc {
    name?: string; //
    lazy?: boolean; // true
    dev?: boolean; // false
}

export class Asset {
    readonly writer = new BytesWriter();
    priority: number = 0;
    owner!: AssetBuilderContext;

    constructor(readonly base_desc: AssetDesc, readonly typeName: string) {

    }

    preBuild(): null | Promise<void> {
        return null;
    }

    build(): null | Promise<void> {
        return null;
    }

    postBuild(): null | Promise<void> {
        return null;
    }

    // generate hash from all inputs to import this asset to resolve if result could be cached
    resolveInputs(): number {
        return H(JSON.stringify(this.base_desc));
    }
}


export class AssetBuilderContext {
    readonly writer = new BytesWriter();

    cache: string = "";
    output: string = "";
    devMode = true;
    assets: Asset[] = [];
    assetsToBuild: Asset[] = [];
    map = new Map<string, Map<string, Asset>>();

    constructor(readonly project: Project, public basePath: string) {

    }

    register(asset: Asset) {
        const type = asset.typeName;
        let m = this.map.get(type);
        if (!m) {
            m = new Map();
            this.map.set(type, m);
        }
        m.set(asset.base_desc.name!, asset);
    }

    find(type: string, name: string): Asset | undefined {
        return this.map.get(type)?.get(name);
    }

    async runPhase(assets: Asset[], phase: "preBuild" | "build" | "postBuild") {
        logger.info(`assets: ${phase}`);
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

    async populate() {
        let total = 0;
        if (isDir(this.basePath)) {
            const bp = fs.realpathSync(this.basePath);
            const scripts = expandGlobSync(path.join(bp, "**/assets.{js,ts}"));
            for (const script of scripts) {
                const ctx = await import(script.path);
                if (ctx.on_populate) {
                    ctx.on_populate(this);
                    ++total;
                }
            }
        }
        if (total === 0) {
            logger.warn(`Asset scripts not found (${this.basePath}) ⬇ \n`);
        }
    }

    prepare() {
        // read declaration for all populated assets,
        // and add to queue assets that should be packed (dev / platform)
        // platform filter is not implemented yet
        for (const asset of this.assets) {
            // skip dev files for build without editor
            if (this.devMode || !asset.base_desc.dev) {
                this.assetsToBuild.push(asset);
                this.register(asset);
            }
        }

        this.assetsToBuild.sort((a, b) => a.priority - b.priority);
    }

    cleanOutput() {
        if (isDir(this.output)) {
            fs.rmSync(this.output, {recursive: true});
        }
        ensureDirSync(this.output);
    }

    async buildIfChanged() {
        // TODO: reset cache option

        const checksumFilename = ".inputs_checksum";
        const inputsHashFilepath = path.join(this.output, checksumFilename);

        let inputsHash = H(this.output + "-" + this.project.current_target + "-" + this.devMode);
        for (const asset of this.assetsToBuild) {
            inputsHash ^= asset.resolveInputs();
        }
        const inputsHashString = (inputsHash >>> 0).toString(16);

        if (isFile(inputsHashFilepath)) {
            const hash = readTextFileSync(inputsHashFilepath);
            logger.log("Previous assets hash:", hash);
            if (hash === inputsHashString) {
                logger.info("Assets are not changed! Keep previous result");
                return;
            }
        } else {
            logger.log("Previous assets hash file not found, rebuild assets");
        }

        this.cleanOutput();

        await this.build();

        writeTextFileSync(inputsHashFilepath, inputsHashString);
    }

    async build() {
        ensureDirSync(this.output);

        // then all queued assets should pass 3 steps: before build, build, after build
        await this.runPhase(this.assetsToBuild, "preBuild");
        await this.runPhase(this.assetsToBuild, "build");
        await this.runPhase(this.assetsToBuild, "postBuild");

        for (const a of this.assetsToBuild) {
            if (a.writer.size > 0) {
                this.writer.writeSection(a.writer);
            }
        }
        this.writer.writeU32(0);
        this.writer.save(path.join(this.output, "pack.bin"));
    }
}