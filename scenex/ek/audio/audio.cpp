#include "audio.hpp"

#include <soloud.h>
#include <soloud_speech.h>
#include <soloud_wav.h>
#include <ek/app/res.hpp>
#include <ek/util/assets.hpp>
#include <ek/util/logger.hpp>

namespace ek::audiomini {

SoLoud::Soloud* soloud = nullptr;

void init() {
    soloud = new SoLoud::Soloud();
    soloud->init();
    EK_INFO << soloud->mBackendID;
}

void create_sound(const char* name) {
    EK_INFO << "create sound: " << name;
    std::string path{name};
    get_resource_content_async(name, [path](array_buffer buf) {
        asset_t<SoLoud::Wav> wav{path};
        wav.reset(new SoLoud::Wav);
        auto res = wav->loadMem(buf.data(), buf.size(), true, true);
        EK_INFO << "loaded sound: " << path << " result: " << res;
    });

}

void create_music(const char* name) {

}

void play_sound(const char* name, float vol, float pan) {
    asset_t<SoLoud::Wav> wav{name};
    if (wav) {
        auto res = soloud->play(*wav.get_mutable());
        EK_INFO << "play sound: " << name << " result: " << res;
    } else {
        EK_INFO << "no sound: " << name;
    }
    SoLoud::Speech* speech = new SoLoud::Speech();
//     Configure sound source
    speech->setText("1 2 3   1 2 3   Hello world. Welcome to So-Loud.");
//     Play the sound source (we could do this several times if we wanted)
    soloud->play(*speech);
}

void play_music(const char* name, float vol) {

}

void vibrate(int duration_millis) {

}


}