#pragma once

namespace ek::audio {

void init();

void create_sound(const char* name);

void create_music(const char* name);

void play_sound(const char* name, float vol, float pan);

void play_music(const char* name, float vol);

}
