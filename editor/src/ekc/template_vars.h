#pragma once

#include <kainjow/mustache.hpp>
#include <ek/fs/path.hpp>

namespace ekc {

struct template_vars_t {
    using data_type = kainjow::mustache::data;
    data_type data;
};

void copy_template(const template_vars_t& vars, const ek::path_t& src, const ek::path_t& dest);

}


