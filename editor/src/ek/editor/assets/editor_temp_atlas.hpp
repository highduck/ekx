#pragma once

#include <ek/spritepack/sprite_data.hpp>
#include <scenex/2d/atlas.hpp>
#include <scenex/asset2/asset_object.hpp>
#include <utility>
#include <ek/fs/path.hpp>

namespace ek {

spritepack::atlas_t prepare_temp_atlas(const std::string& name, float scale);

scenex::atlas_t* load_temp_atlas(spritepack::atlas_t& temp_atlas);

bool check_xml_meta_asset(const std::string& type, const path_t& path);

}

namespace scenex {

template<typename T>
class editor_asset_resolver_t : public asset_type_resolver_t {
public:
    explicit editor_asset_resolver_t(std::string type_name)
            : type_{std::move(type_name)} {
    }

    [[nodiscard]]
    scenex::asset_object_t* create_from_file(const std::string& path) const override {
        if (ek::check_xml_meta_asset(type_, project_->base_path / path)) {
            return create(path);
        }
        return nullptr;
    }

    [[nodiscard]]
    scenex::asset_object_t* create(const std::string& path) const override {
        return new T(path);
    }

    [[nodiscard]]
    asset_object_t* create_for_type(const std::string& type, const std::string& path) const override {
        return type_ == type ? create(path) : nullptr;
    };

private:
    std::string type_;
};

}