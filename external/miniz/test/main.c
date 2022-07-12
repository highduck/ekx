#include <stdio.h>
#include <stdlib.h>
#include <miniz.h>

int main(int argc, char* argv[]) {

    printf("miniz version: %s\n", MZ_VERSION);
    printf("zlib version: %s\n", ZLIB_VERSION);

    return EXIT_SUCCESS;
}