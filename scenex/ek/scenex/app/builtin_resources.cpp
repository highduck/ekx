#include "builtin_resources.hpp"
#include "basic_application.hpp"

#include <ek/util/assets.hpp>

#include <ek/graphics/vertex_decl.hpp>
#include <ek/graphics/texture.hpp>

#include <ek/scenex/text/font.hpp>

#include <ek/draw2d/drawer.hpp>
#include <ek/imaging/drawing.hpp>
#include <ek/scenex/text/text_drawer.hpp>
#include <ek/app/device.hpp>

namespace ek {

const char* program_2d_vs_ = "#ifdef GL_ES\n"
                             "precision highp float;\n"
                             "#else\n"
                             "#define lowp\n"
                             "#define mediump\n"
                             "#define highp\n"
                             "#endif\n"
                             "\n"
                             "attribute vec2 aPosition;\n"
                             "attribute mediump vec2 aTexCoord;\n"
                             "attribute lowp vec4 aColorMult;\n"
                             "attribute lowp vec4 aColorOffset;\n"
                             "\n"
                             "uniform mat4 uModelViewProjection;\n"
                             "\n"
                             "varying mediump vec2 vTexCoord;\n"
                             "varying lowp vec4 vColorMult;\n"
                             "varying lowp vec4 vColorOffset;\n"
                             "\n"
                             "void main() {\n"
                             "    vTexCoord = aTexCoord;\n"
                             "    vColorMult = aColorMult;\n"
                             "    vColorOffset = aColorOffset;\n"
                             "    gl_Position = uModelViewProjection * vec4(aPosition, 0.0, 1.0);\n"
                             "}";

const char* program_2d_fs_ = "#ifdef GL_ES\n"
                             "precision mediump float;\n"
                             "#else\n"
                             "#define lowp\n"
                             "#define mediump\n"
                             "#define highp\n"
                             "#endif\n"
                             "\n"
                             "varying vec2 vTexCoord;\n"
                             "varying lowp vec4 vColorMult;\n"
                             "varying lowp vec4 vColorOffset;\n"
                             "\n"
                             "uniform lowp sampler2D uImage0;\n"
                             "\n"
                             "void main() {\n"
                             "    lowp vec4 pixelColor = texture2D(uImage0, vTexCoord);\n"
                             "    pixelColor *= vColorMult;\n"
                             "    gl_FragColor = pixelColor + vColorOffset * pixelColor.wwww;\n"
                             "}";

void create_builtin() {
    using namespace graphics;
    asset_t<texture_t> empty_texture{"empty"};
    empty_texture.reset(new texture_t);
    image_t image_t{4, 4};
    fill_image(image_t, 0xFFFFFFFFu);
    empty_texture->upload(image_t);

    auto pr = new program_t(program_2d_vs_, program_2d_fs_);
    pr->vertex = &vertex_2d::decl;
    asset_t<program_t>{"2d"}
            .reset(pr);
}

}