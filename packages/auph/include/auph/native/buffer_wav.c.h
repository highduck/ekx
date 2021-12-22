#define DR_WAV_IMPLEMENTATION

#include <dr/dr_wav.h>

bool auph_load_file_wav(const char* filepath, auph_buffer_data_source* dest) {
    drwav_uint64 totalFrameCount = 0;
    unsigned int channels;
    unsigned int sampleRate;
    int16_t* data = drwav_open_file_and_read_pcm_frames_s16(filepath, &channels, &sampleRate, &totalFrameCount,
                                                            NULL);
    if (data) {
        dest->data.i16 = data;
        dest->format = AUPH_SAMPLE_FORMAT_I16;
        dest->sample_rate = sampleRate;
        dest->channels = channels;
        dest->length = totalFrameCount;
        dest->reader = auph_select_source_reader(dest->format, dest->channels, false);
        return true;
    }
    return false;
}

bool auph_load_memory_wav(const void* data, uint32_t size, auph_buffer_data_source* dest) {
    drwav_uint64 totalFrameCount = 0;
    unsigned int channels;
    unsigned int sampleRate;
    int16_t* samples = drwav_open_memory_and_read_pcm_frames_s16(data, (size_t) size, &channels, &sampleRate,
                                                                 &totalFrameCount,
                                                                 NULL);
    if (data) {
        dest->data.i16 = samples;
        dest->format = AUPH_SAMPLE_FORMAT_I16;
        dest->sample_rate = sampleRate;
        dest->channels = channels;
        dest->length = totalFrameCount;
        dest->reader = auph_select_source_reader(dest->format, dest->channels, false);
        return true;
    }
    return false;
}

typedef struct auph_stream_wav {
    drwav f;
    uint64_t cursor;
    auph_source_reader_func parentReader;
    float prev[10];
} auph_stream_wav;

auph_mix_sample* auph_read_stream_wav(auph_mix_sample* mix,
                                      const double begin,
                                      const double end,
                                      const double advance,
                                      const auph_buffer_data_source* dataSource,
                                      auph_mix_sample volume) {
    auph_stream_wav* stream = (auph_stream_wav*) dataSource->stream_data;
    const int channels = (int) dataSource->channels;
    static const int BufferFloatsMax = 2048 * 10;
    float buffer[BufferFloatsMax];
    ((auph_samples_data*) &dataSource->data)->buffer = buffer;

    if (stream->cursor != (uint64_t) ceil(begin)) {
        stream->cursor = (uint64_t) ceil(begin);
        drwav_seek_to_pcm_frame(&stream->f, stream->cursor);
    }

    double p = begin;
    int newFrames = (int) ceil(end) - (int) ceil(p);
    const int startOffset = (int) p;
    for (int ch = 0; ch < channels; ++ch) {
        buffer[ch] = stream->prev[ch];
    }

    if (newFrames > BufferFloatsMax / channels - 1) {
        newFrames = BufferFloatsMax / channels - 1;
    }
    const drwav_uint64 framesReady =
            newFrames > 0 ? drwav_read_pcm_frames_f32(&stream->f, newFrames, buffer + channels) : 0;

    if (framesReady > 0) {
        for (int ch = 0; ch < channels; ++ch) {
            stream->prev[ch] = buffer[framesReady * channels + ch];
        }
    }
    double x1 = p - startOffset;
    double x2 = end - startOffset;
    if (x1 < 0.0000001) {
        x1 += 1;
        x2 += 1;
    }
    mix = stream->parentReader(mix, x1, x2, advance, dataSource, volume);

    stream->cursor = (uint64_t) ceil(end);
    ((auph_samples_data*) &dataSource->data)->buffer = NULL;

    return mix;
}

bool auph_open_stream_wav(auph_stream_wav* stream, auph_buffer_data_source* dest) {
    dest->format = AUPH_SAMPLE_FORMAT_F32;
    dest->channels = stream->f.channels;
    dest->sample_rate = stream->f.sampleRate;
    dest->length = stream->f.totalPCMFrameCount;

    dest->stream_data = stream;
    stream->parentReader = auph_select_source_reader(dest->format, dest->channels, false);
    dest->reader = auph_read_stream_wav;
    return true;
}

bool auph_open_file_stream_wav(const char* filepath, auph_buffer_data_source* dest) {
    auph_stream_wav* stream = (auph_stream_wav*) calloc(1, sizeof(auph_stream_wav));
    bool ok = drwav_init_file(&stream->f, filepath, NULL);
    if (!ok) {
        free(stream);
        return false;
    }
    return auph_open_stream_wav(stream, dest);
}

bool auph_open_memory_stream_wav(const void* data, uint32_t size, auph_buffer_data_source* dest) {
    auph_stream_wav* stream = (auph_stream_wav*) calloc(1, sizeof(auph_stream_wav));
    bool ok = drwav_init_memory(&stream->f, data, (size_t) size, NULL);
    if (!ok) {
        free(stream);
        return false;
    }
    return auph_open_stream_wav(stream, dest);
}
