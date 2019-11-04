#include "edges.h"

#include <string>
#include <vector>
#include <ek/flash/doc/edge.h>

#include <pugixml.hpp>

namespace ek::flash {

std::vector<std::string> split(const std::string& str, const char separator) {
    using size_type = std::string::size_type;

    std::vector<std::string> result;
    size_type prevPos = 0;
    size_type pos = 0;

    while ((pos = str.find(separator, pos)) != std::string::npos) {
        std::string substring{str.substr(prevPos, pos - prevPos)};
        result.push_back(substring);
        prevPos = ++pos;
    }

    // Last word
    result.push_back(str.substr(prevPos, pos - prevPos));

    return result;
}

double read_double_hex(const char* str, int len) {
    if (str[0] == '#') {
        std::string m{str, static_cast<size_t>(len)};
        auto parts = split(m, '.');
        m = parts[0];
        if (parts.size() == 1) {
            m += "00";
        } else {
            const std::string x = parts[1];
            m += x;
            if (x.empty()) m += "00";
            else if (x.size() < 2) m += "0";
        }
        uint32_t hex = 0;
        sscanf(m.c_str(), "#%08x", &hex);
        return (*reinterpret_cast<int32_t*>(&hex)) / 255.0f;
    }

    // default floating point format
    return strtof(str, nullptr);
}

double read_twips(const char* str, int n) {
    return read_double_hex(str, n) / 20.0;
}

bool is_whitespace(char c) {
    return c == '\n' || c == '\r' || c == ' ' || c == '\t';
}

bool is_cmd(char c) {
    return c == '!' || c == '|' || c == '/' || c == '[' || c == ']';
}

bool is_eos(char c) {
    return c == '\0';
}

int get_value_length(const char* buf) {
    const char* ptr = buf;
    char c = *ptr;
    while (!is_eos(c) && !is_cmd(c) && !is_whitespace(c)) {
        ++ptr;
        c = *ptr;
    }
    return ptr - buf;
}

void parse_edges(const char* data, std::vector<char>& out_commands, std::vector<double>& out_values) {
    if (!data) {
        return;
    }

    char c = *data;
    while (!is_eos(c)) {
        if (is_whitespace(c)) {

        } else if (is_cmd(c)) {
            out_commands.push_back(c);
        } else {
            int l = get_value_length(data);
            out_values.push_back(read_twips(data, l));
            data += l - 1;
        }

        ++data;
        c = *data;
    }
}

edge_t& operator<<(edge_t& r, const xml_node& node) {
    parse_edges(node.attribute("edges").value(), r.commands, r.values);
    r.fill_style_0 = node.attribute("fillStyle0").as_int();
    r.fill_style_1 = node.attribute("fillStyle1").as_int();
    r.stroke_style = node.attribute("strokeStyle").as_int();
    return r;
}

}