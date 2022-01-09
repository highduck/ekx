function generate_vec_cxx_operators(name, fields, scalar_type = "float") {
    const name_t = name + "_t";
    function list(pattern, del = ", ") {
        let values = [];
        for(const field of fields) {
            values.push(pattern.replace(/\$/g, field));
        }
        return values.join(del);
    }

    return `/// ${name}_t operator overloading
inline static ${name_t} operator-(const ${name_t} a) {
    return {{ ${list("-a.$")} }};
}

inline static ${name_t} operator-(const ${name_t} a, const ${name_t} b) {
    return {{ ${list("a.$ - b.$")} }};
}

inline static ${name_t} operator+(const ${name_t} a, const ${name_t} b) {
    return {{ ${list("a.$ + b.$")} }};
}

inline static ${name_t} operator*(const ${name_t} a, const ${name_t} b) {
    return {{ ${list("a.$ * b.$")} }};
}

inline static ${name_t} operator/(const ${name_t} a, const ${name_t} b) {
    EK_ASSERT( ${list("b.$ != 0", " && ")} );
    return {{ ${list("a.$ / b.$")} }};
}

inline static ${name_t} operator*(const ${name_t} a, const ${scalar_type} s) {
    return {{ ${list("s * a.$")} }};
}

inline static ${name_t} operator*(const ${scalar_type} s, const ${name_t} a) {
    return operator*(a, s);
}

inline static ${name_t} operator/(const ${name_t} a, const ${scalar_type} s) {
    EK_ASSERT(s != 0);
    return {{ ${list("a.$ / s")} }};
}

inline static ${name_t}& operator*=(${name_t}& a, const ${scalar_type} s) {
    ${list("a.$ *= s;", " ")}
    return a;
}

inline static ${name_t}& operator/=(${name_t}& a, const ${scalar_type} s) {
    EK_ASSERT(s != 0);
    ${list("a.$ /= s;", " ")}
    return a;
}

inline static ${name_t}& operator+=(${name_t}& a, const ${name_t} b) {
    ${list("a.$ += b.$;", " ")}
    return a;
}

inline static ${name_t}& operator-=(${name_t}& a, const ${name_t} b) {
    ${list("a.$ -= b.$;", " ")}
    return a;
}

inline static ${name_t}& operator/=(${name_t}& a, const ${name_t} b) {
    EK_ASSERT( ${list("b.$ != 0", " && ")} );
    ${list("a.$ /= b.$;", " ")}
    return a;
}

inline static ${name_t}& operator*=(${name_t}& a, const ${name_t} b) {
    ${list("a.$ *= b.$;", " ")}
    return a;
}

inline static bool operator==(const ${name_t} a, const ${name_t} b) {
    return ${list("a.$ == b.$", " && ")};
}

inline static bool operator!=(const ${name_t} a, const ${name_t} b) {
    return !operator==(a, b);
}

`;
}

let code = "#ifdef __cplusplus\n\n";
code += generate_vec_cxx_operators("vec2", ["x", "y"]);
code += generate_vec_cxx_operators("vec3", ["x", "y", "z"]);
code += generate_vec_cxx_operators("vec4", ["x", "y", "z", "w"]);
code += generate_vec_cxx_operators("vec2i", ["x", "y"], "int");
code += generate_vec_cxx_operators("vec3i", ["x", "y", "z"], "int");
code += generate_vec_cxx_operators("vec4i", ["x", "y", "z", "w"], "int");
code += "#endif // __cplusplus\n";

require("fs").writeFileSync("include/ek/math/vec.hpp", code);
