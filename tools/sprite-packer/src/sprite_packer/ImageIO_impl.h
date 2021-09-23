#pragma once

#include <miniz.h>

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

namespace {

inline unsigned char* iwcompress(unsigned char* data, int data_len, int* out_len, int quality) {
    // uber compression
    quality = 10;
    mz_ulong buflen = mz_compressBound(data_len);
    auto* buf = (unsigned char*) malloc(buflen);
    if (mz_compress2(buf, &buflen, data, data_len, quality)) {
        free(buf);
        return nullptr;
    }
    *out_len = (int) buflen;
    return buf;
}

}

#define STBIW_ZLIB_COMPRESS(a, b, c, d)  ::iwcompress(a,b,c,d)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"

#include <stb/stb_image_write.h>

#pragma clang diagnostic pop

#endif

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <stb/stb_image.h>

#endif