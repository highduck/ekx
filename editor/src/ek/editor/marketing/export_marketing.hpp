#pragma once

#include <string>

#include <ek/util/path.hpp>
#include <ek/flash/doc/flash_doc.hpp>

namespace ek {

struct marketing_export_command_t {
    std::string target;
    ek::path_t output;
};

struct marketing_asset_t {
    ek::path_t input{"assets/res"};
    std::vector<marketing_export_command_t> commands;
};

void process_market_asset(const marketing_asset_t& config);

void runFlashFilePrerender(const std::vector<std::string>& args);
}