import {Asset, AssetBuilderContext} from "./Asset.ts";
import {Project} from "../project.ts";
import {fs, path} from "../../deps.ts";
import {AudioAsset, AudioFile} from "./Audio.ts";
import {StaticFileDescription, StaticFileImporter} from "./StaticFileImporter.ts";
import {DynamicAtlasAsset, DynamicAtlasDesc} from "./DynamicAtlas.ts";
import {TTFAsset, TTFImporterDesc} from "./TTF.ts";
import {TextureAsset, TextureImporterDesc} from "./Texture.ts";
import {ModelAsset, ObjImporterDesc} from "./Model.ts";
import {MultiResAtlasAsset, MultiResAtlasImporterDesc} from "./Atlas.ts";
import {BitmapFontAsset, BMFontDesc} from "./BitmapFont.ts";
import {FlashAsset, FlashDesc} from "./Flash.ts";
import {TranslationsAsset, TranslationsDesc} from "./Translations.ts";

export class AssetBuilder extends AssetBuilderContext {

    constructor(project: Project, basePath: string) {
        super(project, basePath);
    }

    audio(desc: AudioFile) {
        this.add_importer(new AudioAsset(desc));
    }

    glob(pattern: string): string[] {
        const bp = Deno.realPathSync(this.basePath);
        const files = fs.expandGlobSync(path.join(bp, pattern));
        const result: string[] = [];
        for (const file of files) {
            result.push(path.relative(bp, file.path));
        }
        return result;
    }

    copy(desc: StaticFileDescription) {
        this.add_importer(new StaticFileImporter(desc))
    }

    dynamic_atlas(desc: DynamicAtlasDesc) {
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

    add_importer(importer: Asset) {
        importer.owner = this;
        this.assets.push(importer);
    }

    obj(desc: ObjImporterDesc) {
        this.add_importer(new ModelAsset(desc));
    }

    atlas(desc: MultiResAtlasImporterDesc) {
        if (!desc.resolutions) {
            desc.resolutions = [
                {scale: 1},
                {scale: 2},
                {scale: 3},
                {scale: 4},
            ];
        }
        if (!desc.webp) {
            desc.webp = {};
        }
        this.add_importer(new MultiResAtlasAsset(desc));
    }

    bmfont(desc: BMFontDesc) {
        this.add_importer(new BitmapFontAsset(desc));
    }

    fla(desc: FlashDesc) {
        this.add_importer(new FlashAsset(desc));
    }

    translations(desc: TranslationsDesc) {
        this.add_importer(new TranslationsAsset(desc));
    }
}