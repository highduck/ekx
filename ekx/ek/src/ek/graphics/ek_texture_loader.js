mergeInto(LibraryManager.library, {
    texloader_create__deps: ['$GL'],
    texloader_create: function (pBasePath, pUrls, numImages, cubeMap, pma) {
        if (!window.TEXTURE_LOADERS) {
            window.TEXTURE_LOADERS = [null];
        }
        var table = window.TEXTURE_LOADERS;
        var getId = function () {
            var next = table.length;
            for (var i = 1; i < next; ++i) {
                if (!table[i]) {
                    table[i] = {};
                    return i;
                }
            }
            table[next] = {};
            return next;
        };
        var id = getId();
        table[id].total = numImages;
        table[id].loaded = 0;
        table[id].images = [];
        table[id].texture = null;
        table[id].textureID = 0;

        var basePath = pBasePath ? UTF8ToString(pBasePath) : null;
        if (basePath && basePath.charAt(basePath.length - 1) !== "/") {
            basePath += "/";
        }

        for (var i = 0; i < numImages; ++i) {
            var pUrl = HEAPU32[(pUrls >> 2) + i];
            if (pUrl) {
                var url = UTF8ToString(pUrl);
                if (basePath) {
                    url = basePath + url;
                }
                var img = new Image;
                table[id].images[i] = img;
                img.onload = function () {
                    var obj = table[id];
                    ++obj.loaded;
                    obj.progress = (100 * (obj.loaded / obj.total)) | 0;
                    if (obj.loaded === obj.total) {
                        var image0 = obj.images[0];
                        obj.width = image0.width;
                        obj.height = image0.height;
                        var gl = GL.currentContext.GLctx;
                        if (pma) {
                            gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
                        }
                        var texture = gl.createTexture();
                        if (cubeMap) {
                            var faces = [
                                gl.TEXTURE_CUBE_MAP_POSITIVE_X,
                                gl.TEXTURE_CUBE_MAP_NEGATIVE_X,
                                gl.TEXTURE_CUBE_MAP_POSITIVE_Y,
                                gl.TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                gl.TEXTURE_CUBE_MAP_POSITIVE_Z,
                                gl.TEXTURE_CUBE_MAP_NEGATIVE_Z
                            ];
                            gl.bindTexture(gl.TEXTURE_CUBE_MAP, texture);
                            for (var faceIndex = 0; faceIndex < faces.length; ++faceIndex) {
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

                        // bind webgl resource to GL handles
                        var textureID = GL.getNewId(GL.textures);
                        texture.name = textureID;
                        GL.textures[textureID] = texture;

                        table[id].texture = texture;
                        table[id].textureID = textureID;
                    }
                }
                img.src = url;
            }
        }

        return id;
    },
    texloader_get: function (id, pState) {
        var table = window.TEXTURE_LOADERS;
        if (table) {
            var obj = table[id];
            if (obj && pState) {
                var i = pState >> 2;
                HEAP32[i++] = obj.progress;
                HEAP32[i++] = obj.width;
                HEAP32[i++] = obj.height;
                HEAP32[i++] = obj.texture != null ? 2 : 1;
                HEAP32[i++] = obj.textureID;
                return 0;
            }
        }
        return 1;
    },
    texloader_destroy: function (id) {
        var table = window.TEXTURE_LOADERS;
        if (table && table[id]) {
            var obj = table[id];
            if (obj && obj.images) {
                for (var i = 0; i < obj.images.length; ++i) {
                    obj.images[i].onload = null;
                    obj.images[i].src = "";
                }
            }
            table[id] = null;
            return 0;
        }
        return 1;
    }
});