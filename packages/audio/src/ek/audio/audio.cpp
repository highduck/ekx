#include "audio.hpp"

namespace ek {

/** Sound **/

void AudioResource::load(const char* filepath, bool streaming) {
    buffer = auph_load(filepath, streaming ? AUPH_FLAG_STREAM : 0);
}

void AudioResource::unload() {
    if (auph_is_active(buffer.id)) {
        auph_unload(buffer);
        buffer.id = 0;
    }
}

AudioResource::~AudioResource() {
    unload();
}

}
