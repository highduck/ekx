#pragma once

#include "graphics.hpp"
#include <functional>

namespace ek {
class image_t;
}

namespace ek::graphics {

Texture* createTexture(const image_t& image, const char* label = nullptr);
Texture* createTexture(ek::image_t* images[6], const char* label = nullptr);
Texture* createRenderTarget(int width, int height, const char* label = nullptr);
void load_texture_lazy(const char* path, std::function<void(Texture*)> callback);
//void load_texture_cube_lazy(const std::vector<std::string>& path_list, std::function<void(Texture*)> callback);

}