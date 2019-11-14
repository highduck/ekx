#include "texture.hpp"

#if defined(__EMSCRIPTEN__)

#include <emscripten.h>

#else
#include <platform/static_resources.hpp>
#include <ek/imaging/decoder.hpp>
#endif

namespace ek {

#if defined(__EMSCRIPTEN__)

void load_texture_async(const char* path, texture_t* to_texture) {
    EM_ASM({
               var img = new Image();
               img.onload = function()
               {
                   console.log("LOADED!");
                   GLctx.bindTexture(0x0DE1/*GLctx.TEXTURE_2D*/, GL.textures[$1]);
//                   GLctx.pixelStorei(0x9240/*GLctx.UNPACK_FLIP_Y_WEBGL*/, true);
                   GLctx.pixelStorei(GLctx.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
                   GLctx.texImage2D(0x0DE1/*GLctx.TEXTURE_2D*/, 0, 0x1908/*GLctx.RGBA*/, 0x1908/*GLctx.RGBA*/,
                                    0x1401/*GLctx.UNSIGNED_BYTE*/, img);
                   GLctx.pixelStorei(GLctx.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
//                   GLctx.pixelStorei(0x9240/*GLctx.UNPACK_FLIP_Y_WEBGL*/, false);
               };
               img.src = UTF8ToString($0);
           }, path, to_texture->handle());
}

#else

void load_texture_async(const char* path, texture_t* to_texture) {
    get_resource_content_async(path, [to_texture](auto buffer) {
        image_t* image = decode_image_data(buffer);
        if (image) {
            to_texture->upload(*image);
            delete image;
        }
    });
}

#endif

}
