#pragma once


#include <scenex/asset2/asset_object.hpp>
#include <string>
#include <vector>
#include <scenex/3d/static_mesh.hpp>
#include <ek/fs/path.hpp>

namespace ek {

class model_asset_t : public scenex::asset_object_t {
public:
    explicit model_asset_t(std::string path);

    void read_decl();

    void load() override;

    void unload() override;

    void gui() override;

    void export_() override;

    void save() override;

    void export_meta(ek::output_memory_stream& output) override;

private:
    path_t path_;
    std::string name_;

    [[nodiscard]] path_t get_relative_path(const path_t& path) const;
};

}