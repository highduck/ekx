#include "builtin_resources.hpp"

#include <ek/util/assets.hpp>

#include <ek/graphics/vertex_decl.hpp>
#include <ek/graphics/texture.hpp>

#include <ek/draw2d/drawer.hpp>
#include <ek/imaging/drawing.hpp>

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
                             "#if defined(TEXTURE) && defined(COLOR_OFFSET)\n"
                             "varying lowp vec3 vColorOffset;\n"
                             "#endif\n"
                             "\n"
                             "void main() {\n"
                             "    vTexCoord = aTexCoord;\n"
                             "\n"
                             "    #if defined(COLOR_OFFSET)\n"
                             "    #if defined(TEXTURE)\n"
                             "    vColorMult = vec4(aColorMult.xyz * aColorMult.a, (1.0 - aColorOffset.a) * aColorMult.a);\n"
                             "    vColorOffset = aColorOffset.xyz;\n"
                             "    #else\n"
                             "    vColorMult = vec4((aColorMult.xyz + aColorOffset.xyz) * aColorMult.a, (1.0 - aColorOffset.a) * aColorMult.a);\n"
                             "    #endif\n"
                             "    #else\n"
                             "    vColorMult = vec4(aColorMult.xyz * aColorMult.a, aColorMult.a);\n"
                             "    #endif\n"
                             "\n"
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
                             "#if defined(TEXTURE) && defined(COLOR_OFFSET)\n"
                             "varying lowp vec3 vColorOffset;\n"
                             "#endif\n"
                             "\n"
                             "uniform lowp sampler2D uImage0;\n"
                             "\n"
                             "void main() {\n"
                             "    #ifdef TEXTURE\n"
                             "\n"
                             "    #ifdef TEXTURE_ALPHA\n"
                             "    lowp vec4 pixelColor = vColorMult * texture2D(uImage0, vTexCoord).r;\n"
                             "    #else\n"
                             "    lowp vec4 pixelColor = vColorMult * texture2D(uImage0, vTexCoord);\n"
                             "    #endif\n"
                             "\n"
                             "    #ifdef COLOR_OFFSET\n"
                             "    gl_FragColor = pixelColor + vec4(vColorOffset * pixelColor.a, 0.0);\n"
                             "    #else\n"
                             "    gl_FragColor = pixelColor;\n"
                             "    #endif\n"
                             "\n"
                             "    #else\n"
                             "    gl_FragColor = vColorMult;\n"
                             "    #endif\n"
                             "}";

void create_builtin() {
    using namespace graphics;
    Res<texture_t> empty_texture{"empty"};
    empty_texture.reset(new texture_t);
    image_t image_t{4, 4};
    fill_image(image_t, 0xFFFFFFFFu);
    empty_texture->upload(image_t);

    auto pr = new program_t(program_2d_vs_, program_2d_fs_, "#define TEXTURE\n#define COLOR_OFFSET\n");
    pr->vertex = &vertex_2d::decl;
    Res<program_t>{"2d"}
            .reset(pr);

    pr = new program_t(program_2d_vs_, program_2d_fs_, "#define TEXTURE_ALPHA\n#define TEXTURE\n#define COLOR_OFFSET\n");
    pr->vertex = &vertex_2d::decl;
    Res<program_t>{"2d_alpha"}
            .reset(pr);

    pr = new program_t(program_2d_vs_, program_2d_fs_, "#define COLOR_OFFSET\n");
    pr->vertex = &vertex_2d::decl;
    Res<program_t>{"2d_color"}
            .reset(pr);

    pr = new program_t(program_2d_vs_, program_2d_fs_, "#define TEXTURE_ALPHA\n#define TEXTURE\n");
    pr->vertex = &vertex_minimal_2d::decl;
    Res<program_t>{"2d_min"}
            .reset(pr);
}

}