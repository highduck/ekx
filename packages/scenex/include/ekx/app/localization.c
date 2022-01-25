#include "localization.h"
#include <ek/log.h>
#include <ek/assert.h>

#ifdef __cplusplus
extern "C" {
#endif

inline static int cmp_key(const void* a, const void* b) {
    const char* ka = ((const str_pair_t*) a)->k;
    const char* kb = ((const str_pair_t*) b)->k;
    const int r = ka != kb ? strcmp(ka, kb) : 0;
    return r;
}

typedef struct mo_header {
    uint32_t magic;
    uint32_t revision;
    uint32_t strings_num;
    uint32_t offsetOriginalStrings;
    uint32_t offsetTranslatedStrings;
} mo_header;

typedef struct mo_string_header {
    uint32_t length;
    uint32_t offset;
} mo_string_header;

/// load MO file, and map c-string pointers to identifiers
/// sourceData will be preserved
/// specification: https://www.gnu.org/software/gettext/manual/gettext.html#Binaries
bool string_catalog_parse(string_catalog* catalog) {
    const char* buffer = (const char*) catalog->buffer;
    const uint32_t length = catalog->length;
    EK_ASSERT (buffer && length && "mo-file should have data");

    const mo_header* hdr = (mo_header*) buffer;
    EK_ASSERT (hdr->magic == 0x950412DE && "mo-file should be little-endian");
    EK_ASSERT(hdr->revision == 0 && "mo-file supports only 0 revision");
    EK_ASSERT(length >= (hdr->offsetOriginalStrings + 2 * sizeof(uint32_t) * hdr->strings_num) &&
              "enough data for original strings");
    EK_ASSERT(length >= (hdr->offsetTranslatedStrings + 2 * 4 * hdr->strings_num) &&
              "enough data for translated strings");

    catalog->strings = (str_pair_t*) malloc(sizeof(str_pair_t) * hdr->strings_num);
    for (uint32_t i = 0; i < hdr->strings_num; ++i) {
        mo_string_header* original = (mo_string_header*) (buffer + hdr->offsetOriginalStrings) + i;
        mo_string_header* translated = (mo_string_header*) (buffer + hdr->offsetTranslatedStrings) + i;
        if (length < original->offset + original->length ||
            length < translated->offset + translated->length) {
            log_error("mo-file: not enough data for strings repository");
            return false;
        }
        catalog->strings[i].k = buffer + original->offset;
        catalog->strings[i].v = buffer + translated->offset;
    }
    qsort(catalog->strings, hdr->strings_num, sizeof(str_pair_t), cmp_key);
    return true;
}

bool string_catalog_touch(string_catalog* catalog) {
    if (catalog->strings) {
        return true;
    }
    return string_catalog_parse(catalog);
}

const char* string_catalog_get(const string_catalog* catalog, const char* text) {
    str_pair_t key;
    key.k = text;
    uint32_t num = ((mo_header*) catalog->buffer)->strings_num;
    str_pair_t* f = (str_pair_t*) bsearch(&key, catalog->strings, num, sizeof(str_pair_t), cmp_key);
    return f ? f->v : text;
}

void string_catalog_free(string_catalog* catalog) {
    free(catalog->strings);
    catalog->strings = NULL;
}

const char* localize(const char* str) {
    localization_t l = s_localization;
    if (str && *str && l.lang_index < l.lang_num) {
        return string_catalog_get(&l.languages[l.lang_index], str);
    }
    return str;
}

bool set_language(lang_name_t name) {
    localization_t l = s_localization;
    for (int i = 0; i < l.lang_num; ++i) {
        if (memcmp(l.languages[i].name.str, name.str, sizeof(lang_name_t)) == 0) {
            return set_language_index(i);
        }
    }
    return false;
}

bool set_language_index(uint32_t index) {
    if (index < s_localization.lang_num) {
        if (string_catalog_touch(&s_localization.languages[index])) {
            s_localization.lang_index = index;
            return true;
        }
    }
    return false;
}

lang_name_t current_lang_name() {
    const localization_t l = s_localization;
    return l.lang_index < l.lang_num ? l.languages[l.lang_index].name : (lang_name_t) {};
}

void add_lang(lang_name_t name, void* buffer, uint32_t size) {
    EK_ASSERT(s_localization.lang_num < LANG_MAX_COUNT);
    string_catalog cat = {};
    cat.name = name;
    cat.buffer = buffer;
    cat.length = size;

    s_localization.languages[s_localization.lang_num++] = cat;
}

bool is_localized(const char* text) {
    const uint32_t index = s_localization.lang_index;
    const uint32_t num = s_localization.lang_num;
    const string_catalog* langs = s_localization.languages;
    return text && *text && index < num &&
           string_catalog_get(&langs[index], text) != text;
}

localization_t s_localization;

#ifdef __cplusplus
}
#endif
