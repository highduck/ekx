#include "gui_dev_mode.hpp"
#include "gui_window_tree.h"
#include "gui_window_stats.h"
#include "editor_assets.hpp"

namespace ek {

void gui_dev_mode() {

    gui_window_tree();
    gui_window_inspector();
    //gui_draw_stats();
    do_editor_debug_runtime_assets();
    gui_ekc();
}

}