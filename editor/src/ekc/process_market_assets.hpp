#pragma once

#include <ek/flash/doc/flash_file.h>
#include "project_config.hpp"

namespace ekc {

void process_market_asset(const project_config_t& config);
void process_flash_archive_market(const ek::flash::flash_file& file);

}