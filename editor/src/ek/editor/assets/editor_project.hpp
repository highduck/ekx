#pragma once

#include <vector>
#include <ek/util/path.hpp>
#include <unordered_map>

namespace ek {

class editor_asset_t;

class editor_project_t {
public:
    using AssetFactory = std::function<editor_asset_t * (path_t)>;

    editor_project_t();

    ~editor_project_t();

    void update_scale_factor(float scale_factor_);

    void build(const path_t& output) const;

    void load_all();

    void unload_all();

    void add_file(const path_t& path);

    void clear();

    void populate(bool auto_load = true);

    template<typename EditorAssetType>
    void register_asset_factory() {
        type_factory[EditorAssetType::type_name] = [](auto x) { return new EditorAssetType(x); };
    }

    path_t base_path{"../assets"};

    std::vector<editor_asset_t*> assets;
    std::unordered_map<std::string, AssetFactory> type_factory;

    float scale_factor = 2.0f;
    int scale_uid = 2;

    bool devMode = true;
};

}