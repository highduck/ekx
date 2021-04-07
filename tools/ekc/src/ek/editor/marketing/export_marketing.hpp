#pragma once

#include <string>
#include <ek/ds/Array.hpp>
#include <ek/util/path.hpp>
#include <ek/xfl/Doc.hpp>

namespace ek {

struct marketing_export_command_t {
    std::string target;
    ek::path_t output;
};

struct marketing_asset_t {
    ek::path_t input{"assets/res"};
    Array<marketing_export_command_t> commands{};
};

void process_market_asset(const marketing_asset_t& config);

void runFlashFilePrerender(const Array<std::string>& args);
}