#pragma once

#include <ek/serialize/serialize.hpp>
#include <ek/ds/String.hpp>
#include <ek/ds/Array.hpp>

enum image_data_type {
    IMAGE_DATA_NORMAL = 0,
    IMAGE_DATA_CUBE_MAP = 1
};

struct image_path_t {
    char str[128];
};

struct image_data_t {
    uint32_t type;
    uint32_t formatMask = 1;
    uint32_t images_num;
    image_path_t images[6];

    template<typename S>
    void serialize(ek::IO<S>& io) {
        io(type, formatMask);
        io(images_num);
        for(uint32_t i = 0; i < images_num; ++i) {
            io.span(images[i].str, sizeof(image_path_t));
        }
    }
};
