#define DR_MP3_IMPLEMENTATION

#include <dr/dr_mp3.h>

bool auph_load_file_mp3(const char* filepath, auph_buffer_data_source* dest) {
    drmp3_config config = {0};
    drmp3_uint64 totalFrames = 0;
    int16_t* data = drmp3_open_file_and_read_pcm_frames_s16(filepath, &config, &totalFrames, NULL);
    if (data) {
        dest->data.i16 = data;
        dest->format = AUPH_SAMPLE_FORMAT_I16;
        dest->sample_rate = config.sampleRate;
        dest->channels = config.channels;
        dest->length = totalFrames;
        dest->reader = auph_select_source_reader(dest->format, dest->channels, false);
        return true;
    }
    return false;
}

bool auph_load_memory_mp3(const void* data, uint32_t size, auph_buffer_data_source* dest) {
    drmp3_config config = {0};
    drmp3_uint64 totalFrames = 0;
    int16_t* samples = drmp3_open_memory_and_read_pcm_frames_s16(data, (size_t) size, &config, &totalFrames, NULL);
    if (data) {
        dest->data.i16 = samples;
        dest->format = AUPH_SAMPLE_FORMAT_I16;
        dest->sample_rate = config.sampleRate;
        dest->channels = config.channels;
        dest->length = totalFrames;
        dest->reader = auph_select_source_reader(dest->format, dest->channels, false);
        return true;
    }
    return false;
}

typedef struct auph_stream_mp3 {
    drmp3 f;
    uint64_t cursor;
    auph_source_reader_func parentReader;
    float prev[10];
} auph_stream_mp3;

auph_mix_sample* auph_read_stream_mp3(auph_mix_sample* mix,
                                      const double begin,
                                      const double end,
                                      const double advance,
                                      const auph_buffer_data_source* dataSource,
                                      auph_mix_sample volume) {
    auph_stream_mp3* stream = (auph_stream_mp3*) dataSource->stream_data;
    const int channels = (int) dataSource->channels;
    static const int BufferFloatsMax = 2048 * 10;
    float buffer[BufferFloatsMax];
    ((auph_samples_data*) &dataSource->data)->buffer = buffer;

    if (stream->cursor != (uint64_t) ceil(begin)) {
        stream->cursor = (uint64_t) ceil(begin);
        drmp3_seek_to_pcm_frame(&stream->f, stream->cursor);
    }

    double p = begin;
    int newFrames = (int) end - (int) p;
    const int startOffset = (int) p;
    for (int ch = 0; ch < channels; ++ch) {
        buffer[ch] = stream->prev[ch];
    }
    if (newFrames > BufferFloatsMax / channels - 1) {
        newFrames = BufferFloatsMax / channels - 1;
    }
    const drmp3_uint64 framesReady =
            newFrames > 0 ? drmp3_read_pcm_frames_f32(&stream->f, newFrames, buffer + channels) : 0;

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

bool auph_open_stream_mp3(auph_stream_mp3* stream, auph_buffer_data_source* dest) {
    dest->format = AUPH_SAMPLE_FORMAT_F32;
    dest->channels = stream->f.channels;
    dest->sample_rate = stream->f.sampleRate;
    dest->length = drmp3_get_pcm_frame_count(&stream->f);

    dest->stream_data = stream;
    stream->parentReader = auph_select_source_reader(dest->format, dest->channels, false);
    dest->reader = auph_read_stream_mp3;
    return true;
}

bool auph_open_file_stream_mp3(const char* filepath, auph_buffer_data_source* dest) {
    auph_stream_mp3* stream = (auph_stream_mp3*) calloc(1, sizeof(auph_stream_mp3));
    bool ok = drmp3_init_file(&stream->f, filepath, NULL);
    if (!ok) {
        free(stream);
        return false;
    }
    return auph_open_stream_mp3(stream, dest);
}

bool auph_open_memory_stream_mp3(const void* data, uint32_t size, auph_buffer_data_source* dest) {
    auph_stream_mp3* stream = (auph_stream_mp3*) calloc(1, sizeof(auph_stream_mp3));
    bool ok = drmp3_init_memory(&stream->f, data, (size_t) size, NULL);
    if (!ok) {
        free(stream);
        return false;
    }
    return auph_open_stream_mp3(stream, dest);
}
