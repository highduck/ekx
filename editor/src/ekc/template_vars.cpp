#include "template_vars.h"
#include <fstream>

namespace ekc {

void copy_template(const template_vars_t& vars, const ek::path_t& src, const ek::path_t& dest) {
    std::ifstream file{src.str(), std::ifstream::binary};
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();

    kainjow::mustache::mustache tmpl{text};
    text = tmpl.render(vars.data);

    std::ofstream out{dest.str(), std::ofstream::binary | std::ofstream::trunc};
    out.write(text.c_str(), text.size());
    out.flush();
}

}