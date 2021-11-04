import type {LoadRequest} from "./index";

declare global {
    class LibraryManager {
        static library: LibraryManager;
    }

    function mergeInto(library: LibraryManager, module: any): void;

    const HEAP32: Int32Array;
    const HEAPU32: Uint32Array;

    interface EmscriptenGLContext {
        readonly GLctx: WebGLRenderingContext;
    }

    interface EmscriptenGL {
        readonly currentContext: EmscriptenGLContext;
    }

    const GL: EmscriptenGL;
    const TextureLoader: {
        get: any,
        load: any,
        destroy: any
    };
}

const LibTextureLoader = {
    texloader_create__deps: ['$GL'],
    texloader_create: function (pUrls: number, numImages: number, pBasePath: number, pVersion: number, flags: number, formatMask: number): number {
        const req: LoadRequest = {
            urls: [],
            formatMask,
            flags,
            gl: GL.currentContext.GLctx
        };
        if (pBasePath) {
            req.basePath = UTF8ToString(pBasePath);
        }
        if (pVersion) {
            req.version = UTF8ToString(pVersion);
        }

        for (let i = 0; i < numImages; ++i) {
            const pUrl = HEAPU32[(pUrls >> 2) + i];
            if (pUrl) {
                req.urls[i] = UTF8ToString(pUrl);
            }
        }

        return TextureLoader.load(req);
    },
    texloader_get: function (id: number, pState: number): number {
        const obj = TextureLoader.get(id);
        if (obj && pState) {
            let i = pState >>> 2;
            HEAP32[i++] = obj.progress;
            HEAP32[i++] = obj.w;
            HEAP32[i++] = obj.h;
            HEAP32[i++] = obj.texture != null ? 2 : 1;
            HEAP32[i++] = obj.textureID;
            return 0;
        }
        return 1;
    },
    texloader_destroy: function (id: number): number {
        return TextureLoader.destroy(id);
    }
};

mergeInto(LibraryManager.library, LibTextureLoader);