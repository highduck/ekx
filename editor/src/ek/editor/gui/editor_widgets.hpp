#pragma once

namespace ek {

class sprite_t;

class atlas_t;

class font_t;

struct sg_file;

struct editor_project_t;

struct particle_decl;

class editor_asset_t;

void gui_sprite_view(const sprite_t* sprite);

void gui_atlas_view(const atlas_t* atlas);

void gui_font_view(const font_t* font);

void gui_sg_file_view(const sg_file* library);

void gui_asset_object_controls(editor_asset_t* asset);

void gui_asset_project(editor_project_t& project);

void gui_particles(particle_decl* decl);

}
