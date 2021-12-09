#ifndef EK_STRING_H
#define EK_STRING_H

#include <ek/pre.h>
#include <stb/stb_sprintf.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Path utilities
 */

/**
 * Find path extension
 * @param path - null-terminated string with file path
 * @return pointer to extension in original `path` null-terminated string
 *  - `ext` for `a/b/c.d.e.ext` (string view points to the first extension character)
 *  - `` for `a/b/c/d` (string view points to the end of original string)
 *  - nullptr if `path` is nullptr
 */
const char* ek_path_ext(const char* path);

/**
 * Find name from file path
 * @param path - null-terminated string with file path
 * @return pointer to `name` in original `path` null-terminated string
 *  - `c.d.e.ext` for `a/b/c.d.e.ext` (string view points to the first extension character)
 *  - `` for `a/b/c/` (string view points to the end of original string)
 *  - nullptr if `path` is nullptr
 */
const char* ek_path_name(const char* path);

void ek_path_dirname(char* buf, uint32_t size, const char* path);

uint32_t ek_path_join(char* buf, uint32_t size, const char* path1, const char* path2);

/**
 * String utilities
 */

int ek_cstr_replace(char* str, char search, char replace);

/**
 *
 * @param a - string 1 or null
 * @param b - string 2 or null
 * @param mode - 0 regular cmp, 1 - cmp ignore case
 * @return strings are equal
 */
bool ek_cstr_equals(const char* a, const char* b, int mode);

/**
 *
 * @param buf - destination buffer
 * @param buf_size - max chars in buffer
 * @param millis - milliseconds
 * @param flags - 1 - keep zero hours
 */
void ek_cstr_format_timer(char* buf, int buf_size, int millis, int flags);

#ifdef __cplusplus
}
#endif

#endif //EK_STRING_H
