#include "sprite_packer/AtlasPack.h"
#include <ek/log.h>

uint32_t* cast_align_4(void* p) {
    EK_ASSERT(((uintptr_t) p % 4u) == 0);
    return (uint32_t*) p;
}

bitmap_t load_bitmap32(const char* filepath) {
    bitmap_t result = {0};
    FILE* stream = fopen(filepath, "rb");
    if (!stream) {
        return result;
    }
    fseek(stream, 0, SEEK_END);
    size_t size = (size_t) ftell(stream);
    uint8_t* buf = (uint8_t*) malloc(size);
    fseek(stream, 0, SEEK_SET);
    fread(buf, size, 1u, stream);
    bool success = ferror(stream) == 0;
    fclose(stream);
    if (success && size < 0x10000000u) {
        int channels = 0;
        result.pixels = (color_t*) cast_align_4(
                stbi_load_from_memory(buf, (int) size, &result.w, &result.h, &channels,
                                      STBI_rgb_alpha)
        );
    }
    free(buf);
    return result;
}

char* freadline(FILE* f, char* buf, uint32_t max) {
    char* res = fgets(buf, (int)max, f);
    if (res) {
        const uint32_t len = strlen(res);
        if (len && res[len - 1] == '\n') {
            res[len - 1] = 0;
        }
        return res;
    }
    buf[0] = 0;
    return 0;
}

// output_path
// atlas_name
// %u(num_resolutions)
// %f %u %u
// ..
// %u(num_inputs)
// input_path
// ...
int exportAtlas(const char* filepath) {
    FILE* f1 = fopen(filepath, "rb");
    if (!f1) {
        log_error("atlas config not found %s", filepath);
        return 1;
    }
    char output_path[1024];
    char atlas_name[128];
    freadline(f1, output_path, sizeof output_path);
    freadline(f1, atlas_name, sizeof atlas_name);
    uint32_t num_res = 0;
    fscanf(f1, "%u\n", &num_res);
    atlas_res_t resolutions[8] = {0};
    uint32_t resolutions_num = 0;
    for (uint32_t i = 0; i < num_res; ++i) {
        resolution_config_t* res = &resolutions[i].resolution;
        fscanf(f1, "%f %hu %hu\n", &res->scale, &res->max_width, &res->max_height);
        resolutions[i].sprites_num = 0;
        ++resolutions_num;
    }

    char sprite_name[128];
    char image_path[1024];
    char input_path[1024];

    uint32_t num_inputs = 0;
    fscanf(f1, "%u\n", &num_inputs);
    for (uint32_t input = 0; input < num_inputs; ++input) {
        freadline(f1, input_path, sizeof input_path);
        if (input_path[0]) {
            FILE* f = fopen(input_path, "rb");
            uint32_t resIndex = 0;
            while (!feof(f)) {
                atlas_res_t* res = resolutions + resIndex;
                uint32_t images_num = 0;
                fscanf(f, "%u\n", &images_num);
                if (res->sprites) {
                    res->sprites = (sprite_data_t*) realloc(res->sprites,
                                                            sizeof(sprite_data_t) * (res->sprites_num + images_num));
                } else {
                    res->sprites = (sprite_data_t*) malloc(sizeof(sprite_data_t) * images_num);
                }
                for (uint32_t i = 0; i < images_num; ++i) {
                    sprite_data_t sprite = {0};
                    uint32_t padding = 0;
                    uint32_t flags = 0;
                    freadline(f, sprite_name, sizeof sprite_name);
                    freadline(f, image_path, sizeof image_path);
                    fscanf(f, "%f %f %f %f %u %u\n", &sprite.rc.x, &sprite.rc.y, &sprite.rc.w, &sprite.rc.h,
                           &padding, &flags);
                    // TODO: print to the global Hash table
                    sprite.name = H(sprite_name);
                    sprite.padding = padding;
                    sprite.flags = flags;
                    sprite.bitmap = load_bitmap32(image_path);
                    if (sprite.bitmap.pixels) {
                        int w = sprite.bitmap.w;
                        int h = sprite.bitmap.h;
                        sprite.source = (irect_t) {0, 0, w, h};
                        res->sprites[res->sprites_num++] = sprite;
                    } else {
                        EK_ASSERT(!"failed to load bitmap");
                    }
                }
                ++resIndex;
            }
            fclose(f);
        } else {
            log_error("<images> @path is empty");
        }
    }
    fclose(f1);

    packAndSaveMultiThreaded(resolutions, resolutions_num, atlas_name, output_path);
    return 0;
}

int main(int argc, char** argv) {
    log_init();
    if (argc < 2) {
        return 1;
    }
    const char* command = argv[1];
    log_info("run command %s", command);
    if(!strcmp("sprite-packer", command)) {
        if (argc < 3) {
            return 1;
        }
        const char* configPath = argv[2];
        return exportAtlas(configPath);
    }
    return 0;
}
