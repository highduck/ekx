#pragma once

#include <cstddef>
#include <cstdint>

namespace ek {

class FileView {
public:
    explicit FileView(const char* path);
    ~FileView();

    [[nodiscard]]
    inline const uint8_t* data() const {
        return data_;
    }

    inline size_t size() const {
        return size_;
    }

private:
    uint8_t* data_ = nullptr;
    size_t size_ = 0;
    bool mapped_ = false;
};

}

