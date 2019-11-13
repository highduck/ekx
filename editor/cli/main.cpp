#include <ek/system/working_dir.hpp>
#include <ek/system/system.hpp>
#include <ek/logger.hpp>
#include <ek/editor/marketing/export_marketing.hpp>

namespace ek {

void main() {
    EK_INFO << "exe: " << get_executable_path();
    EK_INFO << "cwd: " << current_working_directory();

    process_market_asset(marketing_asset_t{});
}

}

void ek_main() {

}