#pragma once

#include <string>
#include <ek/serialize/streams.hpp>

namespace ek {

class asset_manager_t;

enum class AssetObjectState {
    Initial = 0,
    Loading = 1,
    Ready = 2
};

class asset_object_t {
    friend class asset_manager_t;

public:
    asset_object_t() = default;

    virtual ~asset_object_t() = default;

    virtual void load() = 0;

    virtual void poll() {}

    virtual void unload() = 0;

    AssetObjectState state{AssetObjectState::Initial};
    int error = 0;

    [[nodiscard]]
    virtual float getProgress() const {
        return state == AssetObjectState::Ready ? 1.0f : 0.0f;
    }

protected:
    asset_manager_t* project_ = nullptr;
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