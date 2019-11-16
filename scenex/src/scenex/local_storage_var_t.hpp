#pragma once

#include <platform/user_preferences.hpp>
#include <platform/application.hpp>
#include <ek/signals.hpp>
#include <string>

namespace scenex {

class local_storage_var_t {
public:
    ek::signal_t<local_storage_var_t&> changed;

    local_storage_var_t() = delete;

    explicit local_storage_var_t(std::string key, int default_value = 0)
            : key_{std::move(key)} {
        value_ = ek::get_user_preference(key_, default_value);
    }

    int value() const {
        return value_;
    }

    void value(int v) {
        if (value_ != v) {
            value_ = v;
            ek::set_user_preference(key_, v);
            changed.emit(*this);
        }
    }

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
    std::string key_;
    int value_;
};

}


