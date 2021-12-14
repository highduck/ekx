#if !defined(_WIN32) && !defined(_WIN64) && !defined(__EMSCRIPTEN__)
#define EK_HAS_MAP_FILE
#endif

#ifdef EK_HAS_MAP_FILE

#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

// MapFile
// Return the contents of the specified file as a read-only pointer.
//
// Enter:inPathName is a UNIX-style “/”-delimited pathname
//
// Exit:    outDataPtra     pointer to the mapped memory region
//          outDataLength   size of the mapped memory region
//          return value    an errno value on error (see sys/errno.h)
//                          or zero for success
//
static int ek_local_res__map_file(const char* inPathName, uint8_t** outDataPtr, size_t* outDataLength) {
    // Return safe values on error.
    int outError = 0;
    *outDataPtr = NULL;
    *outDataLength = 0;

    // Open the file.
    int fileDescriptor = open(inPathName, O_RDONLY, 0);
    if (fileDescriptor < 0) {
        outError = errno;
    } else {
        struct stat statInfo = {0};
        // We now know the file exists. Retrieve the file size.
        if (fstat(fileDescriptor, &statInfo) != 0) {
            outError = errno;
        } else {
            // Map the file into a read-only memory region.
            *outDataPtr = (uint8_t*)mmap(NULL,
                               statInfo.st_size,
                               PROT_READ,
                               MAP_FILE | MAP_PRIVATE,
                               fileDescriptor,
                               0);
            if (*outDataPtr == MAP_FAILED) {
                outError = errno;
            } else {
                // On success, return the size of the mapped file.
                *outDataLength = statInfo.st_size;
            }
        }

        // Now close the file. The kernel doesn’t use our file descriptor.
        close(fileDescriptor);
    }

    return outError;
}

#endif // EK_HAS_MAP_FILE
