#pragma once

#include <ek/ds/Array.hpp>
#include <ek/util/Path.hpp>
#include <unordered_map>
#include <functional>

namespace ek {

class editor_asset_t;

class editor_project_t {
public:
    using AssetFactory = std::function<editor_asset_t * (path_t)>;

    editor_project_t();

    ~editor_project_t();

    void build(const path_t& output) const;

    void add_file(const path_t& path);

    void clear();

    void populate();

    template<typename EditorAssetType>
    void register_asset_factory() {
        type_factory[EditorAssetType::type_name] = [](auto x) { return new EditorAssetType(x); };
    }

    path_t base_path{"../assets"};

    Array<editor_asset_t*> assets{};
    std::unordered_map<std::string, AssetFactory> type_factory;

    bool devMode = true;
};

}