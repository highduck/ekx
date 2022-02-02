#include <ek/app.h>

#include <ek/gfx.h>
#include <ek/canvas.h>
#include <ek/rnd.h>
#include <ek/math.h>
#include <ek/time.h>
#include <ek/log.h>

//#ifdef __EMSCRIPTEN__
//#include <emscripten.h>
//
//void js_on_draw(void) {
//    EM_ASM({if(this.on_draw) this.on_draw();});
//}
//extern "C" {
//    void canvas_quad_color(float x, float y, float w, float h, color_t color) EMSCRIPTEN_KEEPALIVE;
//}
//void js_initialize() {
//    log_info("js initialized");
//}
//
//#else

#include <quickjs.h>
#include <quickjs-libc.h>

JSRuntime* rt;
JSContext* ctx;


static JSValue js_canvas_quad(JSContext* ctx, JSValueConst this_val,
                              int argc, JSValueConst* argv) {
    double x, y, w, h;
    uint32_t color;
    JS_ToFloat64(ctx, &x, argv[0]);
    JS_ToFloat64(ctx, &y, argv[1]);
    JS_ToFloat64(ctx, &w, argv[2]);
    JS_ToFloat64(ctx, &h, argv[3]);
    JS_ToInt32(ctx, (int32_t*) &color, argv[4]);
    canvas_quad_color((float) x, (float) y, (float) w, (float) h, color_u32(color));
    return JS_UNDEFINED;
}


void js_initialize() {
    rt = JS_NewRuntime();
    EK_ASSERT(rt);
    ctx = JS_NewContext(rt);
    EK_ASSERT(ctx);

    js_std_init_handlers(rt);

    /* loader for ES6 modules */
    JS_SetModuleLoaderFunc(rt, nullptr, js_module_loader, nullptr);

    js_std_add_helpers(ctx, ek_app.argc - 1, ek_app.argv + 1);
/* system modules */
//    js_init_module_std(ctx, "std");
//    js_init_module_os(ctx, "os");

    JSValue global_obj;
    JSValue obj262, obj;

    global_obj = JS_GetGlobalObject(ctx);
    JSValue ff = JS_GetPropertyStr(ctx, global_obj, "print");
    log_warn("%d %p", JS_VALUE_GET_TAG(ff), JS_VALUE_GET_PTR(ff));
    JS_SetPropertyStr(ctx, global_obj, "canvas_quad",
                      JS_NewCFunction(ctx, js_canvas_quad, "canvas_quad", 1));


    const char* str =
            /*
            import * as std from 'std';
            import * as os from 'os';
            globalThis.std = std;
            globalThis.os = os;
            */
            //            "console.log(' [QUICKJS] => Script loaded. Ok. \n');";
            "print('Hello');\n print('Hello');\n print('Hello');\n"
            "for(var n = 1; n <= 5; n++) {\n"
            //            "   print(` [QUICKJS-TRACE] n = ${n}/5 `);\n"
            "    console.log(` [QUICKJS-TRACE] n = ${n}/5 `);\n"
            "}\n"
            "this.on_draw = ()=>{"
            "canvas_quad(20, 20, 100, 100, 0x7FFFFFFF);"
            "print('draw frame');"
            "};"
            //            ""
            "(1 << 5)|0";
    //JS_EVAL_FLAG_COMPILE_ONLY
    JSValue val = JS_EvalThis(ctx, global_obj, str, strlen(str), "<eval>", JS_EVAL_TYPE_GLOBAL | JS_EVAL_FLAG_STRICT);
    if (JS_IsException(val)) {
        JSValue err_file = JS_GetPropertyStr(ctx, val, "fileName");
        JSValue err_line = JS_GetPropertyStr(ctx, val, "lineNumber");
        JSValue err_msg = JS_GetPropertyStr(ctx, val, "message");
        JSValue err_stack = JS_GetPropertyStr(ctx, val, "stack");

//        JS_ToInt32(ctx, &r_error->line, err_line);
//        r_error->message = js_to_string(ctx, err_msg);
//        r_error->file = js_to_string(ctx, err_file);
//        r_error->stack.push_back(js_to_string(ctx, err_stack));
//        r_error->column = 0;

        JS_FreeValue(ctx, err_file);
        JS_FreeValue(ctx, err_line);
        JS_FreeValue(ctx, err_msg);
        JS_FreeValue(ctx, err_stack);
    }
    log_warn("%d %d", JS_VALUE_GET_TAG(val), JS_VALUE_GET_INT(val));
}

void js_on_draw() {
    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue ff = JS_GetPropertyStr(ctx, global_obj, "on_draw");
    JS_Call(ctx, ff, global_obj, 0, nullptr);
}
//#endif


void on_ready() {
    ek_gfx_setup(128);
    canvas_setup();


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

    ek_app.config.title = "js script test";
    ek_app.on_frame = on_frame;
    ek_app.on_ready = on_ready;
}