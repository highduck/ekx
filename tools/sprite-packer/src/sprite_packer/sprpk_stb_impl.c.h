#include <miniz.h>

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

static unsigned char* sprite_pack_compress(unsigned char* data, int data_len, int* out_len, int quality) {
    // uber compression
    quality = 10;
    mz_ulong buf_len = mz_compressBound(data_len);
    uint8_t* buf = (uint8_t*) malloc(buf_len);
    if (mz_compress2(buf, &buf_len, data, data_len, quality)) {
        free(buf);
        return NULL;
    }
    *out_len = (int) buf_len;
    return buf;
}

#define STBIW_ZLIB_COMPRESS(a, b, c, d) sprite_pack_compress(a, b, c, d)

#include <stb/stb_image_write.h>

#endif

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <stb/stb_image.h>

#endif