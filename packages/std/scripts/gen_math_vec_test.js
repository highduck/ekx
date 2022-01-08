function f32_trunc(x) {
    return ((x * 10) | 0) / 10.0;
}

function f32_literal(x) {
    if ((x | 0) === x) return `${x}.0f`;
    return x + "f";
}

function generate_vec_test_suite(name, fields) {
    const name_t = name + "_t";
    const a_values = [];
    const b_values = [];
    for (let i = 0; i < fields.length; ++i) {
        a_values[i] = f32_trunc(100 * Math.random() - 50);
        b_values[i] = f32_trunc(100 * Math.random() - 50);
    }

    function list(pattern, del = ", ") {
        let values = [];
        for (const field of fields) {
            values.push(pattern.replace(/\$/g, field));
        }
        return values.join(del);
    }

    function a() {
        return `${name_t} a = ${name}(${a_values.map(f32_literal).join(" ,")});`;
    }

    function b() {
        return `${name_t} b = ${name}(${b_values.map(f32_literal).join(" ,")});`;
    }

    function checks(op) {
        let values = [];
        for (let i = 0; i < fields.length; ++i) {
            const field = fields[i];
            values.push(`   CHECK_NEAR_EQ(r.${field}, ${f32_literal(op(a_values, b_values, i))});`);
        }
        return values.join("\n");
    }

    const simple_tests = [
        {
            cop: "neg",
            op: "-",
            _eval: (a, b, i) => -a[i],
            binary: false
        },
        {
            cop: "add",
            op: "+",
            _eval: (a, b, i) => a[i] + b[i],
            binary: true
        },
        {
            cop: "sub",
            op: "-",
            _eval: (a, b, i) => a[i] - b[i],
            binary: true
        },
        {
            cop: "mul",
            op: "*",
            _eval: (a, b, i) => a[i] * b[i],
            binary: true
        },
        {
            // TODO:
            //cop: "div",
            op: "/",
            _eval: (a, b, i) => a[i] / b[i],
            binary: true
        },
        {
            cop: "scale",
            c_arg_list: "a, b[0]",
            op_type1: name,
            op_type2: "float",
            op: "*",
            op_expr: "a * b[0]",
            op_inv: "*",
            op_inv_expr: "a[0] * b",
            _eval: (a, b, i) => a[i] * b[0],
            _eval_inv: (a, b, i) => a[0] * b[i],
            binary: true
        }
    ];

    const simple_tests_code = [];
    for (const simple_test of simple_tests) {
        let op = undefined;
        let op_inv = simple_test.op_inv;
        let cop = simple_test.cop;
        let arg_list = "a";
        let op_type1 = simple_test.op_type1 ?? name;
        let op_type2 = simple_test.op_type2 ?? name;
        let op_case_name = op ?? (simple_test.op + name);
        if (simple_test.binary) {
            //op = op ?? "a " + simple_test.op + " b";
            op_case_name = op ?? op_type1 + " " + op + " " + op_type2;
            arg_list = simple_test.c_arg_list ?? "a, b";
        }
        if (cop) {
            simple_tests_code.push(
                `TEST_CASE("${name}_${cop}") {
${a()}
${b()}
    ${name_t} r = ${name}_${cop}(${arg_list});
${checks(simple_test._eval)}
}`);
        }
        if (op) {
            simple_tests_code.push(`TEST_CASE("${op_type1} ${op_inv} ${op_type2}") {
${a()}
${b()}
    ${name_t} r = ${simple_test.op_expr ?? (simple_test.binary ? "a " + op + " b" : op + "a")};
${checks(simple_test._eval)}
}`);
        }
        if (op_inv) {
            simple_tests_code.push(`TEST_CASE("${op_type2} ${op_inv} ${op_type1}") {
${a()}
${b()}
    ${name_t} r = ${simple_test.op_inv_expr ?? "b " + op_inv + " a"};
${checks(simple_test._eval_inv)}
}`);
        }
    }

    return `TEST_SUITE_BEGIN("${name}");

${simple_tests_code.join("\n\n")}

TEST_SUITE_END();
`;
}

let code = `
#ifndef MATH_VEC_TEST_H
#define MATH_VEC_TEST_H

#include "math_test_common.h"

`;
code += generate_vec_test_suite("vec2", ["x", "y"]);
code += generate_vec_test_suite("vec3", ["x", "y", "z"]);
code += generate_vec_test_suite("vec4", ["x", "y", "z", "w"]);
code += "#endif // MATH_VEC_TEST_H\n";

require("fs").writeFileSync("test/math/math_vec_test.h", code);
