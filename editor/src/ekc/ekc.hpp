#pragma once

#include <string>

namespace ekc {

struct project_config_t;
struct asset_entry_config_t;

void build_web(const project_config_t& project);

void create_android_project(const project_config_t& config);

void create_xcode_ios(const project_config_t& config);

void process_assets(const project_config_t& config);

}