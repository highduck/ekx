#include "graphics.hpp"

namespace ek {

/** shader wrapper **/
Shader::Shader(const sg_shader_desc* desc) {
    shader = sg_make_shader(desc);
    numFSImages = desc->fs.images[0].name != nullptr ? 1 : 0;
}

Shader::~Shader() {
    sg_destroy_shader(shader);
}

}
