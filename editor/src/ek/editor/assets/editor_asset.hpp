#pragma once

#include <scenex/asset2/asset_manager.hpp>
#include <scenex/asset2/asset_object.hpp>
#include <ek/fs/path.hpp>
#include <pugixml.hpp>

namespace ek {

class editor_asset_t : public scenex::asset_object_t {
public:
    explicit editor_asset_t(std::string path, std::string type_name);

    virtual void read_decl();

    virtual void read_decl_from_xml(const pugi::xml_node& node);

    virtual void gui();

    virtual void export_();

    virtual void save();

    virtual void export_meta(output_memory_stream& output);

    [[nodiscard]]
    const std::string& get_name() const {
        return name_;
    }

    [[nodiscard]]
    const std::string& get_type_name() const {
        return type_name_;
    }

    [[nodiscard]]
    const path_t& get_path() const {
        return path_;
    }

    [[nodiscard]]
    const path_t& get_resource_path() const {
        return resource_path_;
    }

protected:

    [[nodiscard]]
    path_t get_relative_path(const path_t& path) const;

    path_t resource_path_;
    // todo: rename to declaration path
    path_t path_;
    std::string name_;
    std::string type_name_;
};

}

