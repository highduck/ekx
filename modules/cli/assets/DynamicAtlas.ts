import {Asset, AssetDesc} from "./Asset.js";
import {H} from "../utility/hash.js";

export interface DynamicAtlasDesc extends AssetDesc {
    name: string;
    alpha_map?: boolean; // false
    mipmaps?: boolean; // false
}

export class DynamicAtlasAsset extends Asset {
    static typeName = "dynamic_atlas";

    constructor(readonly desc: DynamicAtlasDesc) {
        super(desc, DynamicAtlasAsset.typeName);
    }

    build(): null {
        let flags = 0;
        if (this.desc.alpha_map) flags |= 1;
        if (this.desc.mipmaps) flags |= 2;

        this.writer.writeU32(H(DynamicAtlasAsset.typeName));
        this.writer.writeU32(H(this.desc.name));
        this.writer.writeU32(flags);

        return null;
    }

}