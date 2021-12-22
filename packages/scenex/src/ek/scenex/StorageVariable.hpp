#pragma once

#include <ek/local_storage.h>
#include <ek/util/Signal.hpp>

namespace ek {

class StorageVariable {
public:
    Signal<StorageVariable&> changed;

    StorageVariable() = delete;

    explicit StorageVariable(const char* key, int default_value = 0) :
            key_{key} {
        value_ = ek_ls_get_i(key, default_value);
    }

    [[nodiscard]]
    int value() const {
        return value_;
    }

    void value(int v) {
        if (value_ != v) {
            value_ = v;
            ek_ls_set_i(key_, v);
            changed.emit(*this);
        }
    }

    [[nodiscard]]
    bool enabled() const {
        return value() != 0;
    }

    void enabled(bool flag) {
        value(flag ? 1 : 0);
    }

    bool toggle() {
        bool v = !enabled();
        enabled(v);
        return v;
    }

private:
    const char* key_;
    int value_;
};

}


