export const enum Flags {
    PremultiplyAlpha = 1,
    CubeMap = 2
}

const webpData = "data:image/webp;base64,UklGRh4AAABXRUJQVlA4TBEAAAAvAAAAAAfQ//73v/+BiOh/AAA=";
let webpSupport = false;

fetch(webpData)
    .then(r => r.blob())
    .then(b => createImageBitmap(b))
    .then(() => webpSupport = true)
    .catch();

const iMask = 0x000FFFF;
const vMask = 0xFFF0000;
const vIncr = 0x0010000;

class Loader {
    id = 0;
    total = 0;
    loaded = 0;
    progress = 0;
    w = 0;
    h = 0;
    images: HTMLImageElement[] = [];
    texture: null | WebGLTexture = null;
    textureID: number = 0;
    gl: null | WebGLRenderingContext = null;
    flags = 0;
}

const loaders: Loader[] = [null as any as Loader];

function genId() {
    const next = loaders.length;
    for (let i = 1; i < next; ++i) {
        if (loaders[i].total === 0) {
            return i;
        }
    }
    loaders[next] = new Loader();
    loaders[next].id = next;
    return next;
}

export interface LoadRequest {
    basePath?: string;
    urls: string[];
    version?: string;
    flags: Flags;
    gl?: WebGLRenderingContext;
    formatMask: number;
}

export function get(id: number): Loader | null {
    const obj = loaders[id];
    return (obj && obj.id === id) ? obj : null;
}

export function destroy(id: number) {
    const loader = loaders[id];
    if (loader && loader.images) {
        for (let i = 0; i < loader.images.length; ++i) {
            const img = loader.images[i];
            if (img) {
                img.onload = null;
                img.src = "";
            }
            loader.images.length = 0;
            loader.texture = null;
            loader.total = 0;
            loader.id = ((loader.id + vIncr) & vMask) + (loader.id & iMask);
        }
        return 0;
    }
    return 1;
}

interface EmscriptenTexture extends WebGLTexture {
    name: number;
}

interface EmscriptenGL {
    currentContext: {
        GLctx: WebGLRenderingContext
    };

    textures: EmscriptenTexture[];

    getNewId(textures: EmscriptenTexture[]): number;
}

declare const GL: EmscriptenGL;

export function load(request: LoadRequest): number {
    const id = genId();
    const loader = loaders[id];
    loader.total = request.urls.length;
    loader.flags = request.flags;
    loader.gl = request.gl || (!!GL ? GL.currentContext.GLctx : null);

    let basePath = request.basePath;
    if (basePath && basePath.charAt(basePath.length - 1) !== "/") {
        basePath += "/";
    }

    const tryWebP = (request.formatMask & 2) !== 0 && webpSupport;

    for (let i = 0; i < loader.total; ++i) {
        let url = request.urls[i];
        if (url) {
            if (tryWebP) {
                let lastDotIndex = url.lastIndexOf(".");
                if (lastDotIndex >= 0) {
                    url = url.substring(0, lastDotIndex) + ".webp";
                }
            }

            if (basePath) {
                url = basePath + url;
            }
            let img = new Image();
            loader.images[i] = img;
            img.onload = function () {
                const obj = loaders[id];
                ++obj.loaded;
                obj.progress = (100 * (obj.loaded / obj.total)) | 0;
                if (obj.loaded === obj.total) {
                    const image0 = obj.images[0];
                    obj.w = image0.width;
                    obj.h = image0.height;
                    const gl = obj.gl;
                    if (gl) {
                        const pma = !!(obj.flags & Flags.PremultiplyAlpha);
                        if (pma) {
                            gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
                        }
                        const cubeMap = !!(obj.flags & Flags.CubeMap);
                        const texture = gl.createTexture() as EmscriptenTexture;
                        if (cubeMap) {
                            const faces = [
                                gl.TEXTURE_CUBE_MAP_POSITIVE_X,
                                gl.TEXTURE_CUBE_MAP_NEGATIVE_X,
                                gl.TEXTURE_CUBE_MAP_POSITIVE_Y,
                                gl.TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                gl.TEXTURE_CUBE_MAP_POSITIVE_Z,
                                gl.TEXTURE_CUBE_MAP_NEGATIVE_Z
                            ];
                            gl.bindTexture(gl.TEXTURE_CUBE_MAP, texture);
                            for (let faceIndex = 0; faceIndex < faces.length; ++faceIndex) {
                                gl.texImage2D(faces[faceIndex], 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, obj.images[faceIndex]);
                            }
                            gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
                            gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
                            gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
                            gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
                        } else {
                            gl.bindTexture(gl.TEXTURE_2D, texture);
                            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image0);
                            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
                            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
                            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
                            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
                            gl.bindTexture(gl.TEXTURE_2D, null);
                        }

                        if (pma) {
                            gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
                        }

                        obj.texture = texture;

                        // bind webgl resource to Emscripten's GL handles
                        if (!!GL) {
                            const textureID = GL.getNewId(GL.textures);
                            texture.name = textureID;
                            GL.textures[textureID] = texture;
                            obj.textureID = textureID;
                        }
                    }
                }
            }
            img.src = url;
        }
    }

    return id;
}