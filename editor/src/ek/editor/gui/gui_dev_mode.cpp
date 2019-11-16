#include "gui_dev_mode.h"
#include "gui_window_tree.h"
#include "gui_window_stats.h"
#include "editor_assets.hpp"

namespace ek {

void gui_dev_mode() {

    scenex::gui_window_tree();
    scenex::gui_window_inspector();
    //scenex::gui_draw_stats();
    scenex::do_editor_debug_runtime_assets();
    gui_ekc();
}

}