#include "graphics.hpp"
#include <ek/math/Rect.hpp>

namespace ek {

/** shader wrapper **/
Shader::Shader(const sg_shader_desc* desc) {
    shader = sg_make_shader(desc);
    numFSImages = desc->fs.images[0].name != nullptr ? 1 : 0;
}

Shader::~Shader() {
    sg_destroy_shader(shader);
}

/** texture wrapper **/

Texture::Texture(const sg_image_desc& desc) {
    image = sg_make_image(desc);
}

Texture::Texture(sg_image image_) : image{image_} {
}

Texture::~Texture() {
    sg_destroy_image(image);
}

void Texture::update(const void* data, uint32_t size) const {
    sg_image_data imageData{};
    imageData.subimage[0][0].ptr = data;
    imageData.subimage[0][0].size = size;
    sg_update_image(image, imageData);
}

Texture* Texture::solid(int width, int height, uint32_t pixelColor) {
    return new Texture(ek_gfx_make_color_image(width, height, pixelColor));
}

bool Texture::getPixels(void* pixels) const {
    return ek_gfx_read_pixels(image, pixels);
}

Texture* Texture::renderTarget(int width, int height, const char* label) {
    return new Texture(ek_gfx_make_render_target(width, height, label));
}

}
