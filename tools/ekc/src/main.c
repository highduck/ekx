#include "ek/log.h"
#include "sprite_packer/export_atlas.h"
#include "bmfont_export/bmfont_export.h"
#include "obj-export/obj_export.h"

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
    else if (!strcmp("bmfont-export", command)) {
        if (argc < 3) {
            return 1;
        }
        const char* config_path = argv[2];
        return export_bitmap_font(config_path);
    }
    else if (!strcmp("obj", command)) {
        if (argc < 4) {
            return 1;
        }
        const char* input = argv[2];
        const char* output = argv[3];
        convertObjModel(input, output);
    }
    return 0;
}
