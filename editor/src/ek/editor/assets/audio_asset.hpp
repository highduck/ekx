#pragma once

#include <scenex/asset2/asset_object.hpp>
#include <string>
#include <vector>

namespace ek {

class audio_asset_t : public scenex::asset_object_t {
public:
    explicit audio_asset_t(std::string path);

    void load() override;

    void unload() override;

    void gui() override;

    void export_() override;

    void save() override;

    void export_meta(output_memory_stream& output) override;

private:
    std::string path_;
    std::string name_;
    std::vector<std::string> sound_list_;
    std::vector<std::string> music_list_;
};

}

