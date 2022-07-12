#include <stdio.h>
#include <stdlib.h>

#define DR_MP3_IMPLEMENTATION
#include <dr/dr_mp3.h>

#define DR_WAV_IMPLEMENTATION
#include <dr/dr_wav.h>

#define DR_FLAC_IMPLEMENTATION
#include <dr/dr_flac.h>

int main(int argc, char* argv[]) {

    printf("versions:\n");
    printf("dr_mp3: %s\n", DRMP3_VERSION_STRING);
    printf("dr_wav: %s\n", DRWAV_VERSION_STRING);
    printf("dr_flac: %s\n", DRFLAC_VERSION_STRING);

    int error = 0;
    if (error) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}