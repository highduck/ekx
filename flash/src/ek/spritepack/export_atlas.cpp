#include "export_atlas.hpp"

#include "sprite_packing.hpp"
#include "save_image_png.hpp"

#include <ek/binpack/max_rects.hpp>
#include <ek/timer.hpp>
#include <ek/logger.hpp>
#include <ek/math/serialize_math.hpp>
#include <ek/fs/path.hpp>
#include <ek/fs/system.hpp>

#include <thread>

namespace ek::spritepack {

inline std::string get_atlas_suffix(float scale, int page_index = 0) {
    std::string suffix;
    if (page_index != 0) {
        suffix += "_" + std::to_string(page_index);
    }

    if (scale > 3) suffix += "@4x";
    else if (scale > 2) suffix += "@3x";
    else if (scale > 1) suffix += "@2x";
    return suffix;
}

void save_atlas_resolution(atlas_resolution_t& resolution, const std::string& name) {
    int page_index = 0;
    for (auto& page : resolution.pages) {
        assert(page.image != nullptr);
        page.image_path = name + get_atlas_suffix(resolution.resolution_scale, page_index) + ".png";
        save_image_png_stb(*page.image, page.image_path);
        ++page_index;
    }

    output_memory_stream os{100};
    IO io{os};
    io(const_cast<atlas_resolution_t&>(resolution));
    save(os, path_t{name + get_atlas_suffix(resolution.resolution_scale) + ".atlas"});
}

static void export_atlas_thread(const std::string& name,
                                atlas_resolution_t& resolution
) {
    const std::string log_name = name + "-" + std::to_string(resolution.resolution_index);
    {
        EK_INFO("  - Begin Pack %s", log_name.c_str());
        timer_t timer{};
        resolution.pages = pack(resolution.sprites, resolution.max_size);
        EK_INFO("  - End Pack %s : %0.3f ms", log_name.c_str(), timer.read_millis());
    }
    {
        EK_INFO("  - Begin Save %s", log_name.c_str());
        timer_t timer{};
        save_atlas_resolution(resolution, name);
        EK_INFO("  - End Save %s : %0.3f ms", log_name.c_str(), timer.read_millis());
    }
}

void export_atlas(atlas_t& atlas) {
    timer_t timer{};

    std::vector<std::thread> threads;
    threads.reserve(atlas.resolutions.size());

    for (auto& resolution : atlas.resolutions) {
        threads.emplace_back(&export_atlas_thread, atlas.name, std::ref(resolution));
    }

    for (auto& th : threads) {
        th.join();
    }

    EK_INFO("ATLAS BUILD %s : %0.3f ms", atlas.name.c_str(), timer.read_millis());
}

}