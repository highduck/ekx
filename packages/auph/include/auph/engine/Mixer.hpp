#pragma once

#include "Types.hpp"
#include "../device/AudioDevice.hpp"

//template< bool Interpolate, unsigned Channels, typename S, unsigned Divisor>
//inline auph_mix_sample* auph_mix_samples_T(auph_mix_sample* mix,
//                                           const double begin,
//                                           const double end,
//                                           const double advance,
//                                           const auph_buffer_data_source* audioSource,
//                                           auph_mix_sample volume) {
//    const float gainL = volume.L / Divisor;
//    const float gainR = volume.R / Divisor;
//    const S* source = reinterpret_cast<const S*>(audioSource->data.buffer);
//    double p = begin;
//    while (p < end) {
//        const auto frame = (uint32_t) p;
//        if (Interpolate) {
//            const uint32_t frames = audioSource->length;
//            const auto t = p - (double) frame;
//            const auto ti = 1.0 - t;
//            const auto frame2 = (frame + 1) % frames;
//            if (Channels == 2) {
//                auto iA = frame << 1;
//                auto iB = frame2 << 1;
//                mix->L += gainL * (source[iA++] * ti + source[iB++] * t);
//                mix->R += gainR * (source[iA] * ti + source[iB] * t);
//            } else if (Channels == 1) {
//                const float M = source[frame] * ti + source[frame2] * t;
//                mix->L += gainL * M;
//                mix->R += gainR * M;
//            }
//        } else {
//            if (Channels == 2) {
//                auto i = frame << 1;
//                mix->L += gainL * source[i++];
//                mix->R += gainR * source[i];
//            } else if (Channels == 1) {
//                const float M = source[frame];
//                mix->L += gainL * M;
//                mix->R += gainR * M;
//            }
//        }
//        ++mix;
//        p += advance;
//    }
//    return mix;
//}

auph_mix_sample*
auph_mix_samples_f32_2(auph_mix_sample* mix, const double begin, const double end, const double advance,
                       const auph_buffer_data_source* audioSource, auph_mix_sample volume) {
    const float gainL = volume.L;
    const float gainR = volume.R;
    const float* source = (const float*) audioSource->data.buffer;
    double p = begin;
    while (p < end) {
        const uint32_t frame = (uint32_t) p;
        uint32_t i = frame << 1;
        mix->L += gainL * source[i++];
        mix->R += gainR * source[i];
        ++mix;
        p += advance;
    }
    return mix;
}

auph_mix_sample*
auph_mix_samples_f32_1(auph_mix_sample* mix, const double begin, const double end, const double advance,
                       const auph_buffer_data_source* audioSource, auph_mix_sample volume) {
    const float gainL = volume.L;
    const float gainR = volume.R;
    const float* source = (const float*) audioSource->data.buffer;
    double p = begin;
    while (p < end) {
        const uint32_t frame = (uint32_t) p;
        const float M = source[frame];
        mix->L += gainL * M;
        mix->R += gainR * M;
        ++mix;
        p += advance;
    }
    return mix;
}


auph_mix_sample*
auph_mix_samples_i16_2(auph_mix_sample* mix, const double begin, const double end, const double advance,
                       const auph_buffer_data_source* audioSource, auph_mix_sample volume) {
    const float gainL = volume.L / 0x8000;
    const float gainR = volume.R / 0x8000;
    const int16_t* source = (const int16_t*) audioSource->data.buffer;
    double p = begin;
    while (p < end) {
        const uint32_t frame = (uint32_t) p;
        uint32_t i = frame << 1;
        mix->L += gainL * (float) source[i++];
        mix->R += gainR * (float) source[i];
        ++mix;
        p += advance;
    }
    return mix;
}

auph_mix_sample*
auph_mix_samples_i16_1(auph_mix_sample* mix, const double begin, const double end, const double advance,
                       const auph_buffer_data_source* audioSource, auph_mix_sample volume) {
    const float gainL = volume.L / 0x8000;
    const float gainR = volume.R / 0x8000;
    const int16_t* source = (const int16_t*) audioSource->data.buffer;
    double p = begin;
    while (p < end) {
        const uint32_t frame = (uint32_t) p;
        const float M = (float) source[frame];
        mix->L += gainL * M;
        mix->R += gainR * M;
        ++mix;
        p += advance;
    }
    return mix;
}

void clear(float* dest, uint32_t size) {
    for (uint32_t i = 0; i < size; ++i) {
        dest[i] = 0.0f;
    }
}

void clip(float* dest, uint32_t size) {
    for (uint32_t i = 0; i < size; ++i) {
        const float v = dest[i];
        if (v > 1.0f) {
            dest[i] = 1.0f;
        } else if (v < -1.0f) {
            dest[i] = -1.0f;
        }
    }
}


auph_source_reader_func auph_select_source_reader(auph_sample_format format, uint32_t channels, bool interpolate) {
    if (format == AUPH_SAMPLE_FORMAT_F32) {
        if (channels == 2) {
            return auph_mix_samples_f32_2;
        } else if (channels == 1) {
            return auph_mix_samples_f32_1;
        }
    } else if (format == AUPH_SAMPLE_FORMAT_I16) {
        if (channels == 2) {
            return auph_mix_samples_i16_2;
        } else if (channels == 1) {
            return auph_mix_samples_i16_1;
        }
    }
    return NULL;
}

void auph_render_voices(auph_voice_obj* voices, auph_bus_obj* busline, uint32_t voicesCount, auph_mix_sample* dest,
                        uint32_t frames,
                        uint32_t sampleRate) {
    const float masterGain = auph_bus_get_gain_f(busline);
    for (uint32_t voiceIndex = 0; voiceIndex < voicesCount; ++voiceIndex) {
        auph_voice_obj* voice = voices + voiceIndex;
        if ((voice->state & AUPH_FLAG_RUNNING) && voice->data && voice->data->reader) {
            auph_mix_sample* currentDest = dest;
            const float busGain = auph_bus_get_gain_f(&busline[voice->bus.id & AUPH_MASK_INDEX]) * masterGain;

            double p = voice->position;
            const double pitch = auph_voice_get_rate_f64(voice) * (double) voice->data->sample_rate / sampleRate;
            double playNext = 0.0;
            double playTo = voice->position + frames * pitch;
            const double len = (double) voice->data->length;
            if (playTo >= len) {
                if (voice->state & AUPH_FLAG_LOOP) {
                    playNext = playTo - len;
                } else {
                    voice->state ^= AUPH_FLAG_RUNNING;
                }
                playTo = len;
            }
            voice->position = playTo;
            const float gain = busGain * auph_voice_get_gain_f32(voice);
            const float pan = auph_voice_get_pan_f32(voice);
            const auph_mix_sample volume = {gain * (1.0f - pan),
                                            gain * (1.0f + pan)};
            currentDest = voice->data->reader(currentDest, p, playTo, pitch, voice->data, volume);
            if (playNext > 0.0) {
                voice->position = playNext;
                voice->data->reader(currentDest, 0.0, playNext, pitch, voice->data, volume);
//                voice.position = 0;
            }

            if (!(voice->state & AUPH_FLAG_RUNNING)) {
                auph_voice_obj_stop(voice);
            }
        }
    }
}
