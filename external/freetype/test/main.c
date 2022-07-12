#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>
#include <freetype/freetype.h>

FT_Library library;

int main(int argc, char* argv[]) {
    FT_Error error;
    FT_Int major = 0;
    FT_Int minor = 0;
    FT_Int patch = 0;

    error = FT_Init_FreeType(&library);
    if (error)
        return EXIT_FAILURE;

    FT_Library_Version(library, &major, &minor, &patch);
    if (major != FREETYPE_MAJOR
        || minor != FREETYPE_MINOR
        || patch != FREETYPE_PATCH)
        return EXIT_FAILURE;

    printf("FT_Library_Version: %d.%d.%d\n", major, minor, patch);

    error = FT_Done_FreeType(library);
    if (error)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}