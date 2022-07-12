#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_vorbis.c>

int main(int argc, char* argv[]) {

    printf("library version: %d.%d.%d\n", 0, 0, 1);

    int error = 0;
    if (error) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}