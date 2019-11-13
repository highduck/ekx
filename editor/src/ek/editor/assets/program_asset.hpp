#pragma once

#include <scenex/asset2/asset_object.hpp>
#include <string>
#include <vector>

namespace ek {

class program_asset_t : public scenex::asset_object_t {
public:
    explicit program_asset_t(std::string path);

    void load() override;

    void unload() override;

    void gui() override;

    void export_() override;

    void save() override;

    void export_meta(ek::output_memory_stream& output) override;

private:
    std::string path_;
    std::string name_;

    std::string frag_;
    std::string vert_;
    std::string vertex_decl_;
};

}