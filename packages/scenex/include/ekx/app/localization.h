#ifndef EKX_APP_LOCALIZATION_H
#define EKX_APP_LOCALIZATION_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Localization module, works with MO Files (binary PO files)
// inspirations:
// - https://github.com/elnormous/ouzel/tree/master/engine/localization
// - https://github.com/j-jorge/libintl-lite

#define LANG_CODE_MAX_LENGTH (8)
#define LANG_MAX_COUNT (16)

typedef struct lang_name_t {
    char str[LANG_CODE_MAX_LENGTH];
} lang_name_t;

typedef struct str_pair_t {
    const char* k;
    const char* v;
} str_pair_t;

typedef struct string_catalog {
    lang_name_t name;
    const void* buffer;
    uint32_t length;
    str_pair_t* strings;
} string_catalog;

bool string_catalog_touch(string_catalog* catalog);

// returns translated c-string or original text if not found
const char* string_catalog_get(const string_catalog* catalog, const char* text);

void string_catalog_free(string_catalog* catalog);

bool set_language(lang_name_t name);
bool set_language_index(uint32_t index);

lang_name_t current_lang_name();

const char* localize(const char* str);

void add_lang(lang_name_t name, void* buffer, uint32_t size);

bool is_localized(const char* text);

typedef struct localization_t {
    string_catalog languages[LANG_MAX_COUNT];
    uint32_t lang_num;
    uint32_t lang_index;
} localization_t;

extern localization_t s_localization;

#ifdef __cplusplus
}
#endif

#endif // EKX_APP_LOCALIZATION_H
