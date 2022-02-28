
#include "js_api.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <quickjs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void js_canvas_quad_color(float a1, float a2, float a3, float a4, uint32_t a5);


#ifdef __EMSCRIPTEN__
void js_register(void) {
    EM_ASM({
        this["canvas_quad_color"] = _js_canvas_quad_color;

   });
}
#else
extern JSContext* qjs_ctx;

static JSValue qjs_canvas_quad_color(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
  double a1;
  JS_ToFloat64(ctx, &a1, argv[0]);
  double a2;
  JS_ToFloat64(ctx, &a2, argv[1]);
  double a3;
  JS_ToFloat64(ctx, &a3, argv[2]);
  double a4;
  JS_ToFloat64(ctx, &a4, argv[3]);
  int32_t a5;
  JS_ToInt32(ctx, &a5, argv[4]);
  JSValue result = JS_UNDEFINED;
  js_canvas_quad_color((float)a1, (float)a2, (float)a3, (float)a4, (uint32_t)a5);
  return result;
}


void js_register(void) {
    JSValue global_object = JS_GetGlobalObject(qjs_ctx);
   JS_SetPropertyStr(qjs_ctx, global_object, "canvas_quad_color", JS_NewCFunction(qjs_ctx, qjs_canvas_quad_color, "canvas_quad_color", 1));
}


#endif

#ifdef __cplusplus
}
#endif

