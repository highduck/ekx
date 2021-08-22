#pragma once

#include <string>
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

    virtual void load() = 0;

    virtual void poll() {}

    virtual void unload() = 0;

    AssetState state = AssetState::Initial;
    int error = 0;

    [[nodiscard]]
    virtual float getProgress() const {
        return state == AssetState::Ready ? 1.0f : 0.0f;
    }

protected:
    AssetManager* project_ = nullptr;
};

class AssetTypeResolver {
    friend class AssetManager;

public:
    virtual ~AssetTypeResolver() = default;

    [[nodiscard]]
    virtual Asset* create_from_file(const std::string& path) const = 0;

    [[nodiscard]]
    virtual Asset* create(const std::string& path) const = 0;

    [[nodiscard]]
    virtual Asset* create_for_type(const std::string& type, const std::string& path) const = 0;

    AssetManager* manager = nullptr;
};

class AssetManager {
public:
    AssetManager();

    ~AssetManager();

    void add_file(const std::string& path);

    Asset* add_from_type(const std::string& type, const std::string& path);

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
    Asset* create_from_file(const std::string& path) const override;

    [[nodiscard]]
    Asset* create(const std::string& path) const override;

    [[nodiscard]]
    Asset* create_for_type(const std::string& type, const std::string& path) const override;
};

}