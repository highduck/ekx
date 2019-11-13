#pragma once

#include <string>

#include <ek/fs/path.hpp>
#include <ek/flash/doc/flash_file.h>

namespace ek {

struct marketing_asset_t {
    std::string name{"res"};
    ek::path_t input{"assets"};
};

void process_market_asset(const marketing_asset_t& config);

}