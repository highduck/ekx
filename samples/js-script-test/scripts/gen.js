let functions = {
    canvas_quad_color: {
        sig: ["void", "f32", "f32", "f32", "f32", "u32"]
    }
};

let decl_emscripten = `void js_register(void) {
    EM_ASM({\n`;
for(let fn of Object.keys(functions)) {
    decl_emscripten += `        this["${fn}"] = _js_${fn};\n`;
}
decl_emscripten += `
   });
}`;

let proxy_qjs = ``;

const convert_arg_type_qjs = {
    void: "void",
    f32: "float",
    f64: "double",
    i32: "int32_t",
    u32: "uint32_t"
};

const convert_type_qjs = {
    void: "void",
    f32: "double",
    i32: "int32_t",
    u32: "int32_t"
};

const convert_function_qjs = {
    void: "void",
    f32: "JS_ToFloat64",
    u32: "JS_ToInt32"
};

let fwd_js_functions = ``;
for(let fn of Object.keys(functions)) {
    const sig = functions[fn].sig;
    const ret = sig[0];

    fwd_js_functions += `extern ${ret} js_${fn}(`;
    let args = [];
    for (let i = 1; i < sig.length; ++i) {
        args.push(convert_arg_type_qjs[sig[i]] + " a" + i);
    }
    fwd_js_functions += args.length === 0 ? "void" : args.join(", ");
    fwd_js_functions += ");\n";
}

for(let fn of Object.keys(functions)) {
    const sig = functions[fn].sig;
    const ret = sig[0];

    proxy_qjs += `static JSValue qjs_${fn}(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {\n`;
    let values = [];
    for(let i = 1; i < sig.length; ++i) {
        proxy_qjs += `  ${convert_type_qjs[sig[i]]} a${i};\n`;
        proxy_qjs += `  ${convert_function_qjs[sig[i]]}(ctx, &a${i}, argv[${i - 1}]);\n`;
        values.push("(" + convert_arg_type_qjs[sig[i]] + ")a" + i);
    }

    proxy_qjs += `  JSValue result = JS_UNDEFINED;\n`;
    proxy_qjs += "  ";
    if(ret !== "void") {
        proxy_qjs += "result = ";
    }
    proxy_qjs += `js_${fn}(${values.join(", ")});\n`;
    proxy_qjs += "  return result;\n";
    proxy_qjs += "}\n";
}

let decl_qjs = `void js_register(void) {
    JSValue global_object = JS_GetGlobalObject(qjs_ctx);
`;
for(let fn of Object.keys(functions)) {
    decl_qjs += `   JS_SetPropertyStr(qjs_ctx, global_object, "${fn}", JS_NewCFunction(qjs_ctx, qjs_${fn}, "${fn}", 1));\n`;
}
decl_qjs += `}\n\n`;

const header = `
#ifndef EK_JS_API_H
#define EK_JS_API_H

#ifdef __cplusplus
extern "C" {
#endif

void js_register(void);

#ifdef __cplusplus
}
#endif

#endif // EK_JS_API_H
`;

const impl = `
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

${fwd_js_functions}

#ifdef __EMSCRIPTEN__
${decl_emscripten}
#else
extern JSContext* qjs_ctx;

${proxy_qjs}

${decl_qjs}
#endif

#ifdef __cplusplus
}
#endif

`;

const fs = require("fs");
const path = require("path");
fs.writeFileSync(path.join(__dirname, "../src/js_api.h"), header, "utf8");
fs.writeFileSync(path.join(__dirname, "../src/js_api.c"), impl, "utf8");