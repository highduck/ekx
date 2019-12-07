#include "bitmap.h"

#include <ek/flash/doc/flash_archive.h>
#include <ek/flash/doc/bitmap.h>

#include <ek/serialize/serialize.hpp>
#include <ek/util/logger.hpp>

#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES

#include <miniz/miniz.h>

namespace ek::flash {

struct bitmap_desc_t {
    uint16_t stride;
    uint16_t width;
    uint16_t height;
    uint32_t width_high;
    uint32_t width_tw;
    uint32_t height_high;
    uint32_t height_tw;
    uint8_t alpha; // flags
    uint8_t compressed; // flags

    template<typename S>
    void serialize(IO<S> io) {
        io(
                stride,
                width,
                height,
                width_high,
                width_tw,
                height_high,
                height_tw,
                alpha,
                compressed
        );
    }
};

// bgra_to_argb / vica versa
void reverse_color_components(uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; i += 4) {
        const auto a = data[i + 0];
        const auto r = data[i + 1];
        const auto g = data[i + 2];
        const auto b = data[i + 3];
        data[i + 0] = b;
        data[i + 1] = g;
        data[i + 2] = r;
        data[i + 3] = a;
    }
}

size_t uncompress(input_memory_stream& input, uint8_t* dest, size_t dest_size) {
    auto chunkSize = input.read<uint16_t>();
    output_memory_stream buffer{chunkSize};
    while (chunkSize > 0u) {
        buffer.write(static_cast<const std::byte*>(input.data()) + input.position(), chunkSize);
        input.seek(chunkSize);
        chunkSize = input.read<uint16_t>();
    }

    size_t sz = dest_size;
    mz_uncompress(dest, reinterpret_cast<mz_ulong*>(&sz), static_cast<const uint8_t*>(buffer.data()), buffer.size());
    return sz;
}

bitmap_t* load_bitmap(const basic_entry& entry) {
    auto* bitmap_ptr = new bitmap_t;
    auto& bitmap = *bitmap_ptr;
    const std::string& data = entry.content();
    input_memory_stream input{data.data(), data.size()};
    IO io{input};
    const auto tag1 = input.read<uint8_t>();
    const auto tag2 = input.read<uint8_t>();
    if (tag1 == 0x3 && tag2 == 0x5) {
        bitmap.path = entry.path();
        bitmap_desc_t desc{};
        io(desc);
        assert(desc.width_tw == desc.width * 20u);
        assert(desc.height_tw == desc.height * 20u);
        bitmap.width = desc.width;
        bitmap.height = desc.height;
        bitmap.bpp = desc.stride / bitmap.width;
        bitmap.alpha = desc.alpha != 0;
        bitmap.data.resize(bitmap.width * bitmap.height * bitmap.bpp);
        const auto bm_size = bitmap.data.size();
        auto* bm_data = bitmap.data.data();
        if (desc.compressed != 0) {
            auto written = uncompress(input, bm_data, bm_size);
            assert(written == bm_size);
        } else {
            input.read(bm_data, bm_size);
        }

        reverse_color_components(bm_data, bm_size);

    } else {
        EK_ERROR << "unsupported dat";
    }

    return bitmap_ptr;
}

}