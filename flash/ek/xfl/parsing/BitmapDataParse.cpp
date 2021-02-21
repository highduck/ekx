#include "parsing.hpp"

#include <ek/xfl/types.hpp>

#include <ek/serialize/serialize.hpp>
#include <ek/util/logger.hpp>

#include <ek/imaging/decoder.hpp>
#include <ek/imaging/image.hpp>
#include <miniz.h>

namespace ek::xfl {

// thanks to https://github.com/charrea6/flash-hd-upscaler/blob/master/images.py
const uint16_t SIGNATURE_JPEG = 0xD8FF;
const uint16_t SIGNATURE_ARGB = 0x0503;
const uint16_t SIGNATURE_CLUT = 0x0303;

struct BitmapItemHeader {
    uint16_t stride;
    uint16_t width;
    uint16_t height;
    uint32_t width_high;
    uint32_t width_tw;
    uint32_t height_high;
    uint32_t height_tw;
    uint8_t flags;

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
                flags
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

// abgr_to_argb / vica versa
void convert_rgba_to_bgra(uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; i += 4) {
        const auto r = data[i + 0];
        const auto b = data[i + 2];
        data[i + 0] = b;
        data[i + 2] = r;
    }
}

// abgr_to_argb / vica versa
void convert_clut(uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; i += 4) {
        const auto r = data[i + 1];
        const auto b = data[i + 3];
        data[i + 1] = b;
        data[i + 3] = r;
    }
}

size_t ekUncompress(input_memory_stream& input, uint8_t* dest, size_t dest_size) {
    auto chunkSize = input.read<uint16_t>();
    output_memory_stream buffer{chunkSize};
    while (chunkSize > 0u) {
        buffer.write(static_cast<const std::byte*>(input.data()) + input.position(), chunkSize);
        input.seek(chunkSize);
        chunkSize = input.read<uint16_t>();
    }

    size_t sz = dest_size;
    mz_uncompress(dest, reinterpret_cast<mz_ulong*>(&sz), static_cast<const uint8_t*>(buffer.data()),
                  (mz_ulong) buffer.size());
    return sz;
}

void readBitmapARGB(input_memory_stream& input, BitmapData& bitmap) {
    BitmapItemHeader desc{};
    IO io{input};
    io(desc);
    const auto compressed = input.read<uint8_t>();
    assert(desc.width_tw == desc.width * 20u);
    assert(desc.height_tw == desc.height * 20u);
    bitmap.width = desc.width;
    bitmap.height = desc.height;
    bitmap.bpp = desc.stride / bitmap.width;
    bitmap.alpha = desc.flags != 0;
    bitmap.data.resize(bitmap.width * bitmap.height * bitmap.bpp);
    const auto bm_size = bitmap.data.size();
    auto* bm_data = bitmap.data.data();
    if ((compressed & 1) != 0) {
        auto written = ekUncompress(input, bm_data, bm_size);
        if (written != bm_size) {
            EK_ERROR << "bitmap decompress error";
        }
    } else {
        input.read(bm_data, bm_size);
    }

    reverse_color_components(bm_data, bm_size);
}

void readBitmapCLUT(input_memory_stream& input, BitmapData& bitmap) {
    BitmapItemHeader desc{};
    IO io{input};
    io(desc);
    assert(desc.width_tw == desc.width * 20u);
    assert(desc.height_tw == desc.height * 20u);
    auto nColors = input.read<uint8_t>();
    if (nColors == 0) {
        nColors = 0xFF;
    }

    // read padding to align
    input.read<uint16_t>();

    std::vector<uint32_t> colorTable;
    colorTable.reserve(nColors);
    for (int i = 0; i < nColors; ++i) {
        colorTable.emplace_back(input.read<uint32_t>());
    }
    // convert color table to our cairo format
    convert_rgba_to_bgra(reinterpret_cast<uint8_t*>(colorTable.data()), colorTable.size() * 4);

    if (!colorTable.empty() && (desc.flags & 0x1) != 0) {
        // transparent
        colorTable[0] = 0x0;
    }
    std::vector<uint8_t> buffer;
    buffer.resize(desc.stride * desc.height);
    auto written = ekUncompress(input, buffer.data(), buffer.size());
    if (written != buffer.size()) {
        EK_ERROR << "bitmap decompress error";
    }
    bitmap.width = desc.width;
    bitmap.height = desc.height;
    bitmap.bpp = 4;
    bitmap.alpha = desc.flags != 0;
    bitmap.data.resize(bitmap.width * bitmap.height * 4);
    auto* pixels = reinterpret_cast<uint32_t*>(bitmap.data.data());
    for (size_t i = 0; i < buffer.size(); ++i) {
        pixels[i] = colorTable[buffer[i]];
    }
}

void readBitmapJPEG(const std::string& data, BitmapData& bitmap) {
    auto* image = decode_image_data(data.data(), data.size());
    if (image != nullptr) {
        bitmap.width = image->width();
        bitmap.height = image->height();
        bitmap.bpp = 4;
        bitmap.alpha = false;
        bitmap.data.resize(bitmap.width * bitmap.height * bitmap.bpp);
        memcpy(bitmap.data.data(), image->data(), bitmap.data.size());
        delete image;
        convert_rgba_to_bgra(bitmap.data.data(), bitmap.data.size());
    }
}

BitmapData* BitmapData::parse(const std::string& data) {
    auto* bitmap_ptr = new BitmapData;
    auto& bitmap = *bitmap_ptr;

    input_memory_stream input{data.data(), data.size()};

    const auto sig = input.read<uint16_t>();
    if (sig == SIGNATURE_ARGB) {
        readBitmapARGB(input, bitmap);
    } else if (sig == SIGNATURE_CLUT) {
        readBitmapCLUT(input, bitmap);
    } else if (sig == SIGNATURE_JPEG) {
        readBitmapJPEG(data, bitmap);
    } else {
        EK_ERROR << "unsupported dat";
    }

    return bitmap_ptr;
}

}