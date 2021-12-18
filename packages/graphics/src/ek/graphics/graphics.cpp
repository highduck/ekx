#include "graphics.hpp"
#include <ek/log.h>
#include <ek/assert.h>
#include <ek/math/Rect.hpp>

namespace ek::graphics {

/** buffer wrapper **/
Buffer::Buffer(sg_buffer_type type, const void* data, uint32_t dataSize) {
    sg_buffer_desc desc{};
    desc.type = (sg_buffer_type) type;
    desc.usage = SG_USAGE_IMMUTABLE;
    desc.data.ptr = data;
    desc.data.size = (size_t) dataSize;
    buffer = sg_make_buffer(&desc);
    size = dataSize;
}

Buffer::Buffer(sg_buffer_type type, sg_usage usage, uint32_t maxSize) {
    sg_buffer_desc desc{};
    desc.usage = usage;
    desc.type = (sg_buffer_type) type;
    desc.size = maxSize;
    buffer = sg_make_buffer(&desc);
    if (buffer.id == 0) {
        abort();
    }
    size = maxSize;
}

Buffer::~Buffer() {
    sg_destroy_buffer(buffer);
}

void Buffer::update(const void* data, uint32_t dataSize) {
    size = dataSize;
    sg_range range;
    range.ptr = data;
    range.size = dataSize;
    sg_update_buffer(buffer, range);
//    sg_append_buffer(buffer, data, dataSize);
//    if (size > size_) {
//        sg_update_buffer(handle_, data, size);
//        size_ = size;
//    } else {
//        if (useDataOrphaning) {
//            GL_CHECK(glBufferData(type, size_, nullptr, usage));
//        }
//        GL_CHECK(glBufferSubData(type, 0, size, data));
//    }
}

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
