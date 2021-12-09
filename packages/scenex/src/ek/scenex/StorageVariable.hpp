#pragma once

#include <ek/LocalStorage.hpp>
#include <ek/app/app.hpp>
#include <ek/util/Signal.hpp>
#include <ek/ds/String.hpp>

namespace ek {

class StorageVariable {
public:
    Signal<StorageVariable&> changed;

    StorageVariable() = delete;

    explicit StorageVariable(const char* key, int default_value = 0) :
            key_{key} {
        value_ = get_user_preference(key, default_value);
    }

    [[nodiscard]]
    int value() const {
        return value_;
    }

    void value(int v) {
        if (value_ != v) {
            value_ = v;
            set_user_preference(key_.c_str(), v);
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
    String key_;
    int value_;
};

}


