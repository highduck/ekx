#include "res_sys.hpp"

#if EK_STATIC_RESOURCES_READ_FILE

#include <cassert>
#include <cstdio>

namespace ek::internal {

std::vector<uint8_t> read_file_bytes(const char* path) {
    assert(path);
    std::vector<uint8_t> buffer;
    auto* stream = fopen(path, "rb");
    if (stream) {
        fseek(stream, 0, SEEK_END);
        buffer.resize(static_cast<size_t>(ftell(stream)));
        fseek(stream, 0, SEEK_SET);

        fread(buffer.data(), buffer.size(), 1u, stream);

        if (ferror(stream) != 0) {
            buffer.resize(0);
            buffer.shrink_to_fit();
        }

        fclose(stream);
    }
    return buffer;
}

}

#endif // #if EK_STATIC_RESOURCES_READ_FILE