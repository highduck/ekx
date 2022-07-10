"use strict";
var TextureLoader = (() => {
  var __defProp = Object.defineProperty;
  var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
  var __getOwnPropNames = Object.getOwnPropertyNames;
  var __hasOwnProp = Object.prototype.hasOwnProperty;
  var __export = (target, all) => {
    for (var name in all)
      __defProp(target, name, { get: all[name], enumerable: true });
  };
  var __copyProps = (to, from, except, desc) => {
    if (from && typeof from === "object" || typeof from === "function") {
      for (let key of __getOwnPropNames(from))
        if (!__hasOwnProp.call(to, key) && key !== except)
          __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
    }
    return to;
  };
  var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

  // web/src/index.ts
  var src_exports = {};
  __export(src_exports, {
    Flags: () => Flags,
    destroy: () => destroy,
    get: () => get,
    load: () => load
  });
  var Flags = /* @__PURE__ */ ((Flags2) => {
    Flags2[Flags2["PremultiplyAlpha"] = 1] = "PremultiplyAlpha";
    Flags2[Flags2["CubeMap"] = 2] = "CubeMap";
    return Flags2;
  })(Flags || {});
  var webpData = "data:image/webp;base64,UklGRh4AAABXRUJQVlA4TBEAAAAvAAAAAAfQ//73v/+BiOh/AAA=";
  var webpSupport = false;
  fetch(webpData).then((r) => r.blob()).then((b) => createImageBitmap(b)).then(() => webpSupport = true).catch();
  var iMask = 65535;
  var vMask = 268369920;
  var vIncr = 65536;
  var Loader = class {
    constructor() {
      this.id = 0;
      this.total = 0;
      this.loaded = 0;
      this.progress = 0;
      this.w = 0;
      this.h = 0;
      this.images = [];
      this.texture = null;
      this.textureID = 0;
      this.gl = null;
      this.flags = 0;
    }
  };
  var loaders = [null];
  var nextFree = 1;
  function getPoolObjectAt(i) {
    let obj = loaders[i];
    if (obj === void 0) {
      obj = new Loader();
      obj.id = i + 1;
      loaders[i] = obj;
    }
    return obj;
  }
  function genId() {
    const index = nextFree;
    const obj = getPoolObjectAt(index);
    let id = obj.id;
    nextFree = id & iMask;
    id = index | id & vMask;
    obj.id = id;
    return id;
  }
  function get(id) {
    const obj = loaders[id & iMask];
    return obj && obj.id === id ? obj : null;
  }
  function destroy(id) {
    const obj = get(id);
    if (obj) {
      if (obj.images) {
        for (let i = 0; i < obj.images.length; ++i) {
          const img = obj.images[i];
          if (img) {
            img.onload = null;
            img.src = "";
          }
        }
        obj.images.length = 0;
      }
      obj.texture = null;
      obj.total = 0;
      obj.id = id + vIncr & vMask | nextFree;
      nextFree = id & iMask;
      return 0;
    }
    return 1;
  }
  function load(request) {
    const id = genId();
    const loader = loaders[id & iMask];
    loader.loaded = 0;
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
        img.onload = () => {
          const obj = get(id);
          if (obj) {
            ++obj.loaded;
            obj.progress = 100 * (obj.loaded / obj.total) | 0;
            if (obj.loaded >= obj.total) {
              const image0 = obj.images[0];
              obj.w = image0.width;
              obj.h = image0.height;
              const gl = obj.gl;
              if (gl) {
                const pma = !!(obj.flags & 1 /* PremultiplyAlpha */);
                if (pma) {
                  gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
                }
                const cubeMap = !!(obj.flags & 2 /* CubeMap */);
                const texture = gl.createTexture();
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
                if (!!GL) {
                  const textureID = GL.getNewId(GL.textures);
                  texture.name = textureID;
                  GL.textures[textureID] = texture;
                  obj.textureID = textureID;
                }
              }
            }
          }
        };
        img.src = url;
      }
    }
    return id;
  }
  return __toCommonJS(src_exports);
})();
//# sourceMappingURL=texture-loader.js.map
