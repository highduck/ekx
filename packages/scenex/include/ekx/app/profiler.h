#ifndef EKX_APP_PROFILER_H
#define EKX_APP_PROFILER_H

#include <ek/math.h>
#include <ek/time.h>
#include <ekx/app/game_display.h>

#ifdef __cplusplus
extern "C" {
#endif

// if less or equal 0 - calculate FPS for every frame
#define FPS_MEASUREMENTS_PER_SECONDS (2)

typedef struct fps_counter {
    float average;
    float counter;
    float accum;
} fps_counter;

void fps_counter_update(fps_counter* fps_counter, float dt);

#define PROFILER_FONT H("Cousine-Regular")
#define PROFILE_TRACK_HEIGHT (30.0f)

enum {
    PROFILE_FPS = 0,
    PROFILE_TRIANGLES = 1,
    PROFILE_DRAW_CALLS = 2,
    PROFILE_FILL_RATE = 3,
    PROFILE_FRAME = 4,
    PROFILE_UPDATE = 5,
    PROFILE_RENDER = 6,
    PROFILE_OVERLAY = 7,
    PROFILE_EVENTS = 8,
    PROFILE_END = 9,
    PROFILE_TRACKS_MAX_COUNT = 10,
    PROFILE_TRACK_CAPACITY = 60
};

struct profiler_track {
    const char* name; // ""
    const char* titleFormat;// = "%s: %d";
    float thMin;// = 1.0f / 60.0f;
    float thMax;// = 1.0f / 30.0f;
    float accum;//0
    float value;//0
    float hist[PROFILE_TRACK_CAPACITY];
    int hist_i;

    float min;// = 0.0f;
    float max;// = 33.3f;
};

typedef struct profile_metrics_t {
    struct profiler_track tracks[PROFILE_TRACKS_MAX_COUNT];
    struct fps_counter fps;
    float drawable_area;
    bool enabled;
} profile_metrics_t;

extern profile_metrics_t s_profile_metrics;

void profiler_init(void);
void profiler_update(float dt);
void profiler_draw(const game_display_info* display_info);
void profiler_render_begin(float drawable_area);
void profiler_render_end();
void profiler_add_time(uint32_t metric, float time);

#ifdef __cplusplus
}
#endif

#endif // EKX_APP_PROFILER_H
