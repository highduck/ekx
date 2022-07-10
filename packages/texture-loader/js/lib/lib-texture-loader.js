// web/src/lib.ts
var LibTextureLoader = {
  texloader_create__deps: ["$GL"],
  texloader_create: function(pUrls, numImages, pBasePath, pVersion, flags, formatMask) {
    const req = {
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
  texloader_get: function(id, pState) {
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
  texloader_destroy: function(id) {
    return TextureLoader.destroy(id);
  }
};
mergeInto(LibraryManager.library, LibTextureLoader);
//# sourceMappingURL=lib-texture-loader.js.map
