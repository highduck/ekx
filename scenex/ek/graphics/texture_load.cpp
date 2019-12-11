#include "texture.hpp"

#include <vector>
#include <string>

#if defined(__EMSCRIPTEN__)

#include <emscripten.h>

#else

#include <ek/app/res.hpp>
#include <ek/imaging/decoder.hpp>

#endif

namespace ek {

#if defined(__EMSCRIPTEN__)

void load_texture_lazy(const char* path, texture_t* to_texture) {
    EM_ASM({
               var img = new Image();
               img.onload = function()
               {
                   console.debug("TEXTURE 2D LOADED!");
                   GLctx.bindTexture(0x0DE1/*GLctx.TEXTURE_2D*/, GL.textures[$1]);
//                   GLctx.pixelStorei(0x9240/*GLctx.UNPACK_FLIP_Y_WEBGL*/, true);
                   GLctx.pixelStorei(GLctx.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
                   GLctx.texImage2D(0x0DE1/*GLctx.TEXTURE_2D*/, 0, 0x1908/*GLctx.RGBA*/, 0x1908/*GLctx.RGBA*/,
                                    0x1401/*GLctx.UNSIGNED_BYTE*/, img);
                   GLctx.pixelStorei(GLctx.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
//                   GLctx.pixelStorei(0x9240/*GLctx.UNPACK_FLIP_Y_WEBGL*/, false);
                   //GLctx.bindTexture(0x0DE1/*GLctx.TEXTURE_2D*/, null);
               };
               img.src = UTF8ToString($0);
           }, path, to_texture->handle());
}

void load_texture_cube_lazy(const std::vector<std::string>& path_list, texture_t* to_texture) {
    EM_ASM({
               var imgs = [];
               var counter = 6;
               for (var i = 0; i < 6; ++i) {
                   imgs[i] = new Image();
                   imgs[i].onload = function()
                   {
                       --counter;
                       if (counter == 0) {
                           console.debug("TEXTURE CUBE LOADED!");
                           GLctx.bindTexture(GLctx.TEXTURE_CUBE_MAP, GL.textures[$6]);
//                   GLctx.pixelStorei(0x9240/*GLctx.UNPACK_FLIP_Y_WEBGL*/, true);
                           GLctx.pixelStorei(GLctx.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
                           for (var j = 0; j < 6; ++j) {
                               GLctx.texImage2D(GLctx.TEXTURE_CUBE_MAP_POSITIVE_X + j,
                                                0,
                                                0x1908/*GLctx.RGBA*/,
                                                0x1908/*GLctx.RGBA*/,
                                                0x1401/*GLctx.UNSIGNED_BYTE*/, imgs[j]);
                           }
                           GLctx.pixelStorei(GLctx.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
//                   GLctx.pixelStorei(0x9240/*GLctx.UNPACK_FLIP_Y_WEBGL*/, false);
                          // GLctx.bindTexture(GLctx.TEXTURE_CUBE_MAP, null);
                       }
                   }
               }
               imgs[0].src = UTF8ToString($0);
               imgs[1].src = UTF8ToString($1);
               imgs[2].src = UTF8ToString($2);
               imgs[3].src = UTF8ToString($3);
               imgs[4].src = UTF8ToString($4);
               imgs[5].src = UTF8ToString($5);
           },
           path_list[0].c_str(),
           path_list[1].c_str(),
           path_list[2].c_str(),
           path_list[3].c_str(),
           path_list[4].c_str(),
           path_list[5].c_str(),
           to_texture->handle());
}

#else

void load_texture_lazy(const char* path, texture_t* to_texture) {
    get_resource_content_async(path, [to_texture](auto buffer) {
        image_t* image = decode_image_data(buffer);
        if (image) {
            to_texture->upload(*image);
            delete image;
        }
    });
}

void load_texture_cube_lazy(const std::vector<std::string>& path_list, texture_t* to_texture) {
    int count = path_list.size();
    std::array<ek::image_t*, 6> images{};
    for (int idx = 0; idx < 6; ++idx) {
        get_resource_content_async(path_list[idx].c_str(), [&images, idx, &count, to_texture](auto buffer) {
            images[idx] = decode_image_data(buffer);
            --count;
            if (count == 0) {
                to_texture->upload_cubemap(images);
                for (auto* img : images) {
                    delete img;
                }
            }
        });
    }
}

#endif

}
