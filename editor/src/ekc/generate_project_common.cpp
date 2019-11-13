#include <ek/system/system.hpp>

using namespace ek;

namespace ekc {

void remove_module_cache() {
    if (is_dir("deps")) {
        remove_dir_rec("deps");
    }
}

void update_module_cache() {
//    if (!is_dir("deps/lodepng")) {
//        execute("git clone --depth=1 https://github.com/lvandeve/lodepng.git deps/lodepng");
//    }
}

}