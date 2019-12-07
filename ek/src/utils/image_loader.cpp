#include <ek/imaging/image.hpp>
#include <ek/imaging/decoder.hpp>
#include <ek/util/path.hpp>
#include <ek/util/logger.hpp>
#include <platform/static_resources.hpp>

namespace ek {

image_t* load_image(const path_t& path) {
    image_t* image = nullptr;
    auto buffer = get_resource_content(path.str());
    if (buffer.empty()) {
        EK_DEBUG << "IMAGE resource not found: " << path;
    } else {
        image = decode_image_data(buffer);
    }
    return image;
}

}