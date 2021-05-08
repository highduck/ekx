#pragma once

#include <string>
#include <ecxx/ecxx.hpp>
#include <ek/math/box.hpp>
#include <ek/math/packed_color.hpp>
#include <IconsFontAwesome5.h>

namespace ek {

class Editor;

void guiEditor(Editor& inspector);

std::string getDebugNodePath(ecs::EntityApi e);

/** Widgets **/
struct TextLayerEffect;

void guiTextLayerEffect(TextLayerEffect& layer);

class Sprite;

class Atlas;

class Font;

class SceneFactory;

struct ParticleDecl;

void gui_sprite_view(const Sprite& sprite);

void gui_font_view(const Font& font);

void gui_particles(ParticleDecl& decl);
}
