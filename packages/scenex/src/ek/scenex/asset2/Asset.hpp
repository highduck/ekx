#pragma once

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

protected:
    AssetManager* manager_ = nullptr;
};

class AssetTypeResolver {
    friend class AssetManager;

public:
    virtual ~AssetTypeResolver() = default;

    [[nodiscard]]
    virtual Asset* create_from_file(const String& path, const String& type) const = 0;

    [[nodiscard]]
    virtual Asset* create(const String& path) const = 0;

    [[nodiscard]]
    virtual Asset* create_for_type(const void* data, uint32_t size) const = 0;

    AssetManager* manager = nullptr;
};

class AssetManager {
public:
    AssetManager();

    ~AssetManager();

    Asset* add_file(const char* path, const char* type);

    Asset* add_from_type(const void* data, uint32_t size);

    void add_resolver(AssetTypeResolver* resolver);

    void load_all();

    void unload_all();

    void clear();

    void set_scale_factor(float scale);

    bool is_assets_ready() const;

    String base_path{"assets"};
    Array<Asset*> assets;
    Array<AssetTypeResolver*> resolvers;
    float scale_factor = 2.0f;
    uint8_t scale_uid = 2;
};

class DefaultAssetsResolver : public AssetTypeResolver {
public:

    [[nodiscard]]
    Asset* create_from_file(const String& path, const String& type) const override;

    [[nodiscard]]
    Asset* create(const String& path) const override;

    [[nodiscard]]
    Asset* create_for_type(const void* data, uint32_t size) const override;
};

}