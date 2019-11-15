#pragma once


#include <scenex/asset2/asset_object.hpp>
#include <string>
#include <vector>
#include <ek/fs/path.hpp>

namespace ek {

class texture_asset_t : public scenex::asset_object_t {
public:
    explicit texture_asset_t(std::string path);

    void read_decl();

    void load() override;

    void unload() override;

    void gui() override;

    void export_() override;

    void save() override;

    void export_meta(ek::output_memory_stream& output) override;

private:
    std::string path_;
    std::string name_;
    std::string texture_type_;
    std::vector<std::string> images_;

    path_t get_relative_path(const path_t& path) const;
};

}