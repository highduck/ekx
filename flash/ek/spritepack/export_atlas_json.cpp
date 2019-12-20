#include "export_atlas.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ek::spritepack {

std::string dump_resolution_json(atlas_resolution_t& resolution) {
    json pages = json::array();
    for (auto& res_page : resolution.pages) {
        json sprites = json::array();
        for (auto& spr : res_page.sprites) {
            sprites.emplace_back(json{
                    {"name",  spr.name},
                    {"rc",    {spr.rc.x, spr.rc.y, spr.rc.width, spr.rc.height}},
                    {"uv",    {spr.uv.x, spr.uv.y, spr.uv.width, spr.uv.height}},
                    {"flags", spr.flags}
            });
        }
        pages.emplace_back(json{
                {"width",      res_page.size.x},
                {"height",     res_page.size.y},
                {"image_path", res_page.image_path},
                {"sprites",    sprites}
        });
    }
    json data = {
            {"pages", pages}
    };

    return data.dump(4);
}

}