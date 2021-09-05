#pragma once

#include <string>
#include <ek/serialize/core.hpp>
#include <ek/serialize/streams.hpp>
#include <ek/ds/Array.hpp>
#include <ek/util/Path.hpp>

namespace ek {

class AssetManager;

enum class AssetState {
    Initial = 0,
    Loading = 1,
    Ready = 2
};

class Asset {
    friend class AssetManager;

public:
    Asset() = default;

    explicit Asset(std::string name) :
            name_{std::move(name)} {
    }

    virtual ~Asset() = default;

    virtual void load() {
        if (state == AssetState::Initial) {
            state = AssetState::Loading;
            this->do_load();
        }
    }

    virtual void unload() {
        if (state == AssetState::Ready) {
            this->do_unload();
            state = AssetState::Initial;
        }
    }

    virtual void poll() {}

    virtual void do_load() {}

    virtual void do_unload() {}

    [[nodiscard]]
    virtual float getProgress() const {
        return state == AssetState::Ready ? 1.0f : 0.0f;
    }

    AssetState state = AssetState::Initial;
    int error = 0;
    float weight_ = 1.0f;
    std::string name_;

protected:
    AssetManager* manager_ = nullptr;
};

class AssetTypeResolver {
    friend class AssetManager;

public:
    virtual ~AssetTypeResolver() = default;

    [[nodiscard]]
    virtual Asset* create_from_file(const std::string& path, const std::string& type) const = 0;

    [[nodiscard]]
    virtual Asset* create(const std::string& path) const = 0;

    [[nodiscard]]
    virtual Asset* create_for_type(const void* data, int size) const = 0;

    AssetManager* manager = nullptr;
};

class AssetManager {
public:
    AssetManager();

    ~AssetManager();

    Asset* add_file(const std::string& path, const std::string& type);

    Asset* add_from_type(const void* data, int size);

    void add_resolver(AssetTypeResolver* resolver);

    void load_all();

    void unload_all();

    void clear();

    void set_scale_factor(float scale);

    bool is_assets_ready() const;

    path_t base_path{"assets"};
    Array<Asset*> assets;
    Array<AssetTypeResolver*> resolvers;
    float scale_factor = 2.0f;
    uint8_t scale_uid = 2;
};

class DefaultAssetsResolver : public AssetTypeResolver {
public:

    [[nodiscard]]
    Asset* create_from_file(const std::string& path, const std::string& type) const override;

    [[nodiscard]]
    Asset* create(const std::string& path) const override;

    [[nodiscard]]
    Asset* create_for_type(const void* data, int size) const override;
};

}