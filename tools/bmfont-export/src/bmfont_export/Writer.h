#pragma once

#include <cstdlib>
#include <cstring>

namespace bmfont_export {

class Writer {
public:

    Writer(int capacity) : pos{0}, cap{capacity}, data{nullptr} {
        data = (uint8_t*)malloc(cap);
    }

    ~Writer() {
        free(data);
    }

    void ensure(int l) {
        if (l > cap) {
            while (l > cap) {
                cap <<= 1;
            }
            data = (uint8_t*) realloc(data, cap);
        }
    }

    void writeI32(int32_t i) {
        ensure(pos + 4);
        memcpy(data + pos, (void*) &i, sizeof(int32_t));
        pos += 4;
    }

    void writeString(const std::string& str) {
        int size = (int)str.size();
        ensure(pos + 4 + size);
        writeI32((int32_t)size);
        memcpy(data + pos, str.c_str(), size);
        pos += size;
    }

    int pos = 0;
    int cap = 0;
    uint8_t* data = nullptr;
};

}