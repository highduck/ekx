#pragma once

namespace scenex {

class sprite_t;

class atlas_t;

class font_t;

struct sg_file;

class asset_manager_t;

class asset_object_t;

}

namespace ek {

void gui_sprite_view(const scenex::sprite_t* sprite);

void gui_atlas_view(const scenex::atlas_t* atlas);

void gui_font_view(const scenex::font_t* font);

void gui_sg_file_view(const scenex::sg_file* library);

void gui_asset_object_controls(scenex::asset_object_t* asset);

void gui_asset_project(scenex::asset_manager_t& project);

}
