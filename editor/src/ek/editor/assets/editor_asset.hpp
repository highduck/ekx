#pragma once

#include <ek/scenex/asset2/asset_manager.hpp>
#include <ek/scenex/asset2/asset_object.hpp>
#include <ek/util/path.hpp>
#include <pugixml.hpp>
#include "editor_project.hpp"

namespace ek {

class editor_project_t;

struct assets_build_struct_t {
    path_t output;
    output_memory_stream* pack_meta_stream = nullptr;

    void meta(std::string type, std::string path);
};


// Editor asset is a meta resource,
// - editor asset could be loaded to game from raw resource
// - editor asset could be exported to asset bundle
class editor_asset_t {
public:
    explicit editor_asset_t(path_t path, std::string type_name);

    virtual ~editor_asset_t() = default;

    virtual void read_decl();

    virtual void read_decl_from_xml(const pugi::xml_node& node);

    virtual void load() = 0;

    virtual void unload() = 0;

    // by default we don't need to reload resource (specialized only for graphics resources)
    virtual void onScaleFactorChanged();

    virtual void gui();

    virtual void build(assets_build_struct_t& data) = 0;

    virtual void save();

    [[nodiscard]]
    const std::string& get_name() const {
        return name_;
    }

    [[nodiscard]]
    const std::string& get_type_name() const {
        return type_name_;
    }

    [[nodiscard]]
    const path_t& get_declaration_path() const {
        return declaration_path_;
    }

    [[nodiscard]]
    const path_t& get_resource_path() const {
        return resource_path_;
    }

    editor_project_t* project = nullptr;

protected:

    [[nodiscard]]
    path_t get_relative_path(const path_t& path) const;

    path_t resource_path_;
    path_t declaration_path_;
    std::string name_;
    std::string type_name_;
    bool reloadOnScaleFactorChanged = false;
};

}

