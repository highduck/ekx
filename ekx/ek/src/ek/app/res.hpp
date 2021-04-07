#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <functional>

namespace ek {

using array_buffer = std::vector<uint8_t>;
using get_content_callback_func = std::function<void(array_buffer)>;

void get_resource_content_async(const char* path, const get_content_callback_func& callback);

/*
 TODO:
 FileView:
 - Open, read to buffer, close. Buffer deletes on the end
 - Map file, unmap file
 - fetch from network .. store buffer. Delete buffer
 */

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


