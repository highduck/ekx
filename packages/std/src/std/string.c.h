#include <ek/string.h>
#include <ek/print.h>

#define ek_path_is_del(ch) ((ch) == '/' || (ch) == '\\')

const char* ek_path_ext(const char* path) {
    if (!path) {
        return 0;
    }

    const char* extension = path;
    const char* last = 0;

    // find the last DOT
    while (*extension != '\0') {
        if (*extension == '.') {
            ++extension;
            last = extension;
        }
        ++extension;
    }

    return last ? last : extension;
}

const char* ek_path_name(const char* path) {
    if (!path) {
        return 0;
    }

    const char* it = path;
    const char* last = path;

    // find the last DELIMITER
    while (*it != '\0') {
        const char ch = *it;
        if (ek_path_is_del(ch)) {
            ++it;
            last = it;
        }
        else {
            ++it;
        }
    }

    return last;
}

void ek_path_dirname(char* buf, uint32_t size, const char* path) {
    const char* name = ek_path_name(path);
    uint32_t len = 0;
    if (name && name != path) {
        len = (uint32_t) (name - path - 1);
        if (len > size) {
            len = size - 1;
        }
        strncpy(buf, path, len);
    }
    buf[len] = '\0';
}

uint32_t ek_path_join(char* buf, uint32_t size, const char* path1, const char* path2) {
    char* it = buf;
    char* end = buf + size - 1;
    if (path1 && path1[0]) {
        bool last_delim = false;
        while (it != end) {
            char c = *(path1++);
            if (c == '\0') break;
            *(it++) = c;
            last_delim = ek_path_is_del(c);
        }
        if (it != end && !last_delim) {
            *(it++) = '/';
        }
    }
    if (path2) {
        // skip all DELIMITER characters in start of `path2`
        while (ek_path_is_del(*path2) && *path2 != '\0') {
            ++path2;
        }
        while (it != end) {
            char c = *(path2++);
            if (c == '\0') break;
            *(it++) = c;
        }
    }
    *it = '\0';
    return (uint32_t) (it - buf);
}

/**
* String utilities
*/

// helper for ignore_case comparison
#if (defined(_WIN32) || defined(_WIN64)) && !defined(strcasecmp)
#define ek_strcasecmp stricmp
#else
#define ek_strcasecmp strcasecmp
#endif

bool ek_cstr_equals(const char* a, const char* b, int mode) {
    if (a && b) {
        if (mode == 0) {
            return strcmp(a, b) == 0;
        } else {
            return ek_strcasecmp(a, b) == 0;
        }
    }
    return false;
}

int ek_cstr_replace(char* str, char search, char repl) {
    int replacements = 0;
    if (str) {
        while (*str != '\0') {
            if (*str == search) {
                *str = repl;
                ++replacements;
            }
            ++str;
        }
    }
    return replacements;
}

void ek_cstr_format_timer(char* buf, int buf_size, int millis, int flags) {
    int seconds = (millis + 999) / 1000;
    int hours = seconds / 3600;
    int minutes = (seconds / 60) % 60;
    int secs = seconds % 60;
    if (hours > 0 || (flags & 1) != 0) {
        ek_snprintf(buf, buf_size, "%02d:%02d:%02d", hours, minutes, secs);
    } else {
        ek_snprintf(buf, buf_size, "%02d:%02d", minutes, secs);
    }
}
