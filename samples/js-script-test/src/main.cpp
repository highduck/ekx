#include <ek/app.h>

#include <ek/gfx.h>
#include <ek/canvas.h>
#include <ek/rnd.h>
#include <ek/math.h>
#include <ek/time.h>
#include <ek/log.h>
#include "js_api.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define JS_API EMSCRIPTEN_KEEPALIVE
#else
#define JS_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

JS_API void js_canvas_quad_color(float x, float y, float w, float h, uint32_t color) {
    canvas_quad_color(x, y, w, h, color_u32(color));
}

#ifdef __cplusplus
}
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

void js_on_draw(void) {
    EM_ASM({if(this["on_draw"]) this["on_draw"]();});
}

void js_initialize() {
    log_info("js initialized");
}

void load_script(const char* path) {
    //t.src = AsciiToString($0);
    EM_ASM({
        var t = document.createElement("script");
        t.src = UTF8ToString($0);
        t.onload = function() {
//            document.head.removeChild(t);
        };
        document.head.appendChild(t);
    }, path);
}

#else

#include <quickjs.h>
#include <quickjs-libc.h>
#include <ek/local_res.h>

JSRuntime* qjs_rt;
JSContext* qjs_ctx;

void js_initialize() {
    qjs_rt = JS_NewRuntime();
    EK_ASSERT(qjs_rt);
    qjs_ctx = JS_NewContext(qjs_rt);
    EK_ASSERT(qjs_ctx);

    js_std_init_handlers(qjs_rt);

    /* loader for ES6 modules */
    JS_SetModuleLoaderFunc(qjs_rt, nullptr, js_module_loader, nullptr);

    js_std_add_helpers(qjs_ctx, ek_app.argc - 1, ek_app.argv + 1);
/* system modules */
//    js_init_module_std(ctx, "std");
//    js_init_module_os(ctx, "os");
}

void js_on_draw() {
    JSValue global_obj = JS_GetGlobalObject(qjs_ctx);
    JSValue ff = JS_GetPropertyStr(qjs_ctx, global_obj, "on_draw");
    JS_Call(qjs_ctx, ff, global_obj, 0, nullptr);
}

void load_script(const char* path) {
    ek_local_res_load(
            path,
            [](ek_local_res* lr) {
                if (ek_local_res_success(lr)) {
                    JSValue global_obj = JS_GetGlobalObject(qjs_ctx);
                    char* buffer = (char*) malloc(lr->length + 1);
                    buffer[lr->length] = 0;
                    memcpy(buffer, lr->buffer, lr->length);
                    JSValue val = JS_EvalThis(qjs_ctx, global_obj, buffer, lr->length, "<eval>",
                                              JS_EVAL_TYPE_GLOBAL | JS_EVAL_FLAG_STRICT);
                    if (JS_IsException(val)) {
                        log_warn("load exception: %d %d", JS_VALUE_GET_TAG(val), JS_VALUE_GET_INT(val));
                    }
                    free(buffer);
                }
                ek_local_res_close(lr);
            },
            nullptr
    );
}

#endif

void on_ready() {
    ek_gfx_setup(128);
    canvas_setup();

    load_script("assets/scripts/main.js");
}

void on_frame() {
    const auto width = ek_app.viewport.width;
    const auto height = ek_app.viewport.height;
    if (width > 0 && height > 0) {
        static sg_pass_action pass_action{};
        pass_action.colors[0].action = SG_ACTION_CLEAR;
        const vec4_t fillColor = vec4_color(ARGB(ek_app.config.background_color));
        pass_action.colors[0].value.r = fillColor.x;
        pass_action.colors[0].value.g = fillColor.y;
        pass_action.colors[0].value.b = fillColor.z;
        pass_action.colors[0].value.a = 1.0f;
        sg_begin_default_pass(&pass_action, (int) width, (int) height);

        canvas_new_frame();
        canvas_begin(width, height);

        js_on_draw();

        canvas_end();

        sg_end_pass();
        sg_commit();
    }
}

void ek_app_main() {
    log_init();
    ek_time_init();
    random_seed = ek_time_seed32();
    srand(ek_time_seed32() + 1);

    js_initialize();
    js_register();

    ek_app.config.title = "js script test";
    ek_app.on_frame = on_frame;
    ek_app.on_ready = on_ready;
}