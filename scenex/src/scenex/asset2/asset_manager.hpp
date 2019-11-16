#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <ek/fs/path.hpp>

namespace scenex {

class asset_type_resolver_t;

class asset_object_t;

class asset_manager_t {
public:
    asset_manager_t();

    ~asset_manager_t();

    void add_file(const std::string& path);

    asset_object_t* add_from_type(const std::string& type, const std::string& path);

    void add_resolver(asset_type_resolver_t* resolver);

    void load_all();

    void unload_all();

    void clear();

    void set_scale_factor(float scale);

    bool is_assets_ready() const;

    ek::path_t base_path{"assets"};
    ek::path_t export_path{"assets"};
    std::vector<asset_object_t*> assets;
    std::vector<asset_type_resolver_t*> resolvers;
    float scale_factor = 2.0f;
    uint8_t scale_uid = 2;
};

}

