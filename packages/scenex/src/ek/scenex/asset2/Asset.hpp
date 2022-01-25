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

class AssetManager {
public:
    void add(Asset* asset);

    void load_all();

    void unload_all();

    void clear();

    void set_scale_factor(float scale);

    [[nodiscard]] bool is_assets_ready() const;

    String base_path{"assets"};
    Array<Asset*> assets;
    float scale_factor = 2.0f;
    uint8_t scale_uid = 2;
};


class PackAsset : public Asset {
public:
    explicit PackAsset(String name);
    void do_load() override;
    void do_unload() override;

    void poll() override;
    [[nodiscard]] float getProgress() const override;

    String name_;
    String fullPath_;
    Array<Asset*> assets;
    unsigned assetsLoaded = 0;
    bool assetListLoaded = false;
};

}