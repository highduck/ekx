#pragma once

#include <string>
#include <ek/serialize/streams.hpp>

namespace scenex {

class asset_manager_t;

class asset_object_t {
    friend class asset_manager_t;

public:
    virtual ~asset_object_t() = default;

    virtual void load() = 0;

    virtual void unload() = 0;

protected:
    asset_manager_t* project_ = nullptr;
    bool ready_ = false;
};

class asset_type_resolver_t {
    friend class asset_manager_t;

public:
    virtual ~asset_type_resolver_t() = default;

    [[nodiscard]]
    virtual asset_object_t* create_from_file(const std::string& path) const = 0;

    [[nodiscard]]
    virtual asset_object_t* create(const std::string& path) const = 0;

    [[nodiscard]]
    virtual asset_object_t* create_for_type(const std::string& type, const std::string& path) const = 0;

    asset_manager_t* project_ = nullptr;
};

}