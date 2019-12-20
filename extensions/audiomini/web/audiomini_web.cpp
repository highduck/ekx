extern "C" {

extern void audio_mini_vibrate(int millis);
extern void audio_mini_play_sound(const char* name, float vol, float pan);
extern void audio_mini_play_music(const char* name, float vol);
extern void audio_mini_create_sound(const char* name);
extern void audio_mini_create_music(const char* name);

}

namespace ek::audio_mini {

void create_sound(const char* name) {
    audio_mini_create_sound(name);
}

void create_music(const char* name) {
    audio_mini_create_music(name);
}

void play_sound(const char* name, float vol, float pan) {
    audio_mini_play_sound(name, vol, pan);
}

void play_music(const char* name, float vol) {
    audio_mini_play_music(name, vol);
}

void vibrate(int millis) {
    audio_mini_vibrate(millis);
}

}