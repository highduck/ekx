#pragma once

#include <stb/stb_vorbis.c>

typedef struct auph_stream_ogg {
    stb_vorbis* f;
    uint64_t cursor;
    auph_source_reader_func parentReader;
    float prev[10];
} auph_stream_ogg;

auph_mix_sample* auph_read_stream_ogg(auph_mix_sample* mix,
                                      const double begin,
                                      const double end,
                                      const double advance,
                                      const auph_buffer_data_source* dataSource,
                                      auph_mix_sample volume) {
    auph_stream_ogg* stream = (auph_stream_ogg*) dataSource->stream_data;
    const int channels = (int) dataSource->channels;
    static const int BufferFloatsMax = 2048 * 10;
    float buffer[BufferFloatsMax];
    ((auph_samples_data*) &dataSource->data)->buffer = buffer;

    if (stream->cursor != (uint64_t) ceil(begin)) {
        stream->cursor = (uint64_t) ceil(begin);
        stb_vorbis_seek_frame(stream->f, stream->cursor);
    }

    double p = begin;
    int newFrames = (int) end - (int) p;
    const int startOffset = (int) p;
    buffer[0] = stream->prev[0];
    buffer[1] = stream->prev[1];
    for (int ch = 0; ch < channels; ++ch) {
        buffer[ch] = stream->prev[ch];
    }
    const int framesReady = newFrames > 0 ?
                            stb_vorbis_get_samples_float_interleaved(stream->f,
                                                                     (int) channels,
                                                                     buffer + channels,
                                                                     newFrames * channels) : 0;
    if (framesReady > 0) {
        for (int ch = 0; ch < channels; ++ch) {
            stream->prev[ch] = buffer[framesReady * channels + ch];
        }
        mix = stream->parentReader(mix, p - startOffset, end - startOffset, advance, dataSource, volume);
    }
    ((auph_samples_data*) &dataSource->data)->buffer = NULL;
    stream->cursor = (uint64_t) ceil(end);
    return mix;
}

bool auph_load_ogg(stb_vorbis* ogg, auph_buffer_data_source* dest, bool streaming) {
    uint32_t frames = stb_vorbis_stream_length_in_samples(ogg);
    stb_vorbis_info info = stb_vorbis_get_info(ogg);

    dest->format = AUPH_SAMPLE_FORMAT_F32;
    dest->channels = info.channels;
    dest->sample_rate = info.sample_rate;
    dest->length = frames;

    if (streaming) {
        auph_stream_ogg* streamData = (auph_stream_ogg*) calloc(1, sizeof(auph_stream_ogg));
        dest->stream_data = streamData;
        streamData->f = ogg;
        streamData->parentReader = auph_select_source_reader(dest->format, dest->channels, false);
        dest->reader = auph_read_stream_ogg;
    } else {
        int samples = (int) (info.channels * frames);
        float* data = (float*) malloc(4 * info.channels * frames);
        const int numFrames = stb_vorbis_get_samples_float_interleaved(ogg, info.channels, data, samples);
        stb_vorbis_close(ogg);

        dest->data.f32 = data;
        dest->length = numFrames;
        dest->reader = auph_select_source_reader(dest->format, dest->channels, false);
    }
    return true;
}

bool auph_load_file_ogg(const char* filepath, auph_buffer_data_source* dest, bool streaming) {
    int error = 0;
    auto*ogg = stb_vorbis_open_filename(filepath, &error, NULL);
    if (error != 0 || !ogg) {
        stb_vorbis_close(ogg);
        return false;
    }
    return auph_load_ogg(ogg, dest, streaming);
}

bool auph_load_memory_ogg(const void* data, uint32_t size, auph_buffer_data_source* dest, bool streaming) {
    int error = 0;
    auto*ogg = stb_vorbis_open_memory((const uint8_t*) data, (int) size, &error, NULL);
    if (error != 0 || !ogg) {
        stb_vorbis_close(ogg);
        return false;
    }
    return auph_load_ogg(ogg, dest, streaming);
}
