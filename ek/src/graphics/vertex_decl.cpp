#include "vertex_decl.hpp"

namespace ek {

vertex_decl_t vertex_2d::decl{sizeof(vertex_2d), 2, false};
vertex_decl_t vertex_3d::decl{sizeof(vertex_3d), 3, true};

}