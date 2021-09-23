#pragma once

#include <cstdlib>
#include <cstring>

namespace sprite_packer {

class Writer {
public:

    Writer(int capacity) : pos{0}, cap{capacity}, data{nullptr} {
        data = (uint8_t*) malloc(cap);
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

    void writeU8(uint8_t i) {
        ensure(pos + 1);
        data[pos++] = i;
    }

    void writeF32(float i) {
        ensure(pos + 4);
        memcpy(data + pos, (void*) &i, sizeof(float));
        pos += 4;
    }

    void writeI32(int32_t i) {
        ensure(pos + 4);
        memcpy(data + pos, (void*) &i, sizeof(int32_t));
        pos += 4;
    }

    void writeU16(uint16_t i) {
        ensure(pos + 2);
        memcpy(data + pos, (void*) &i, sizeof(uint16_t));
        pos += 2;
    }

    void writeString(const std::string& str) {
        int size = (int) str.size();
        ensure(pos + 4 + size);
        writeI32((int32_t) size);
        memcpy(data + pos, str.c_str(), size);
        pos += size;
    }

    int pos = 0;
    int cap = 0;
    uint8_t* data = nullptr;
};

}