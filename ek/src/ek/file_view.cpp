#include "file_view.hpp"

#ifndef __EMSCRIPTEN__

#include <cerrno>
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
int MapFile(const char* inPathName, void** outDataPtr, size_t* outDataLength) {
    int outError;
    int fileDescriptor;
    struct stat statInfo;

    // Return safe values on error.
    outError = 0;
    *outDataPtr = nullptr;
    *outDataLength = 0;

    // Open the file.
    fileDescriptor = open(inPathName, O_RDONLY, 0);
    if (fileDescriptor < 0) {
        outError = errno;
    } else {
        // We now know the file exists. Retrieve the file size.
        if (fstat(fileDescriptor, &statInfo) != 0) {
            outError = errno;
        } else {
            // Map the file into a read-only memory region.
            *outDataPtr = mmap(NULL,
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

#endif // __EMSCRIPTEN__
namespace ek {

FileView::FileView(const char* path) {
#ifndef __EMSCRIPTEN__
    if (MapFile(path, reinterpret_cast<void**>(&data_), &size_) == 0) {
        mapped_ = true;
    }
#endif
}

FileView::~FileView() {
    if (mapped_) {
#ifndef __EMSCRIPTEN__
        munmap(data_, size_);
#endif
    }
}

}