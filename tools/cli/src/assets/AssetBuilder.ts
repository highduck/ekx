import {Asset, AssetBuilderContext} from "./Asset";
import {Project} from "../project";
import * as path from "path";
import * as fs from "fs";
import * as glob from "glob";
import {AudioAsset, AudioFile} from "./Audio";
import {StaticFileDescription, StaticFileImporter} from "./StaticFileImporter";
import {DynamicAtlasAsset, DynamicAtlasDesc} from "./DynamicAtlas";
import {TTFAsset, TTFImporterDesc} from "./TTF";
import {TextureAsset, TextureImporterDesc} from "./Texture";
import {ModelAsset, ObjImporterDesc} from "./Model";
import {MultiResAtlasAsset, MultiResAtlasImporterDesc} from "./Atlas";
import {BitmapFontAsset, BMFontDesc} from "./BitmapFont";
import {FlashAsset, FlashDesc} from "./Flash";
import {TranslationsAsset, TranslationsDesc} from "./Translations";

export class AssetBuilder extends AssetBuilderContext {

    constructor(project: Project, basePath: string) {
        super(project, basePath);
    }

    audio(desc:AudioFile) {
        this.add_importer(new AudioAsset(desc));
    }

    glob(pattern: string):string[] {
        const bp = fs.realpathSync(this.basePath);
        const files = glob.sync(path.join(bp, pattern));
        for (let i = 0; i < files.length; ++i) {
            files[i] = path.relative(this.basePath, files[i]);
        }
        return files;
    }

    copy(desc:StaticFileDescription) {
        this.add_importer(new StaticFileImporter(desc))
    }

    dynamic_atlas(desc:DynamicAtlasDesc) {
        this.add_importer(new DynamicAtlasAsset(desc));
    }

    default_glyph_cache() {
        this.add_importer(new DynamicAtlasAsset({
            name: "default_glyph_cache",
            alpha_map: true,
            mipmaps: true
        }));
    }

    ttf(desc: TTFImporterDesc) {
        this.add_importer(new TTFAsset(desc));
    }

    texture(desc: TextureImporterDesc) {
        this.add_importer(new TextureAsset(desc));
    }

    add_importer(importer:Asset) {
        importer.owner = this;
        this.assets.push(importer);
    }

    obj(desc:ObjImporterDesc) {
        this.add_importer(new ModelAsset(desc));
    }

    atlas(desc:MultiResAtlasImporterDesc) {
        if(!desc.resolutions) {
            desc.resolutions = [
                {scale: 1},
                {scale: 2},
                {scale: 3},
                {scale: 4},
            ];
        }
        if(!desc.webp) {
            desc.webp = {};
        }
        this.add_importer(new MultiResAtlasAsset(desc));
    }

    bmfont(desc:BMFontDesc) {
        this.add_importer(new BitmapFontAsset(desc));
    }

    fla(desc:FlashDesc) {
        this.add_importer(new FlashAsset(desc));
    }

    translations(desc:TranslationsDesc) {
        this.add_importer(new TranslationsAsset(desc));
    }
}