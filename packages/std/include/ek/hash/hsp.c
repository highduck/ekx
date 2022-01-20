#include <ek/hash.h>
#include <ek/assert.h>

/**
 * in general this huge string table used for checking hash collisions in runtime.
 * also it provides ability to get C-string by Hash value for tooling and debugging.
 * 0 hash value is default and invalid, always points to NULL pointer
 * TODO: collision assert should search next seed value and print it before actual abort
 */

#ifndef NDEBUG

typedef char hsp_debug_string_buffer[64];

static struct {
    uint16_t spatial[0x10000];
    hsp_debug_string_buffer table[0x10000];
    uint16_t next[0x10000];
    uint16_t half_hashes[0x10000];
    uint32_t last;
} hsp;

uint16_t hsp_find(string_hash_t hash_val) {
    uint16_t root = hsp.spatial[hash_val & 0xFFFFu];
    uint16_t it = root;
    while (it != 0) {
        if (hsp.half_hashes[it] == (hash_val >> 16u)) {
            return it;
        }
        it = hsp.next[it];
    }
    return it;
}

void hsp_insert(string_hash_t hash_val, uint16_t id) {
    const uint16_t hash_low = hash_val & 0xFFFFu;
    const uint16_t root = hsp.spatial[hash_low];
    hsp.next[id] = root;
    hsp.spatial[hash_low] = id;
    hsp.half_hashes[id] = hash_val >> 16u;
}

string_hash_t hsp_hash_debug(const char* str) {
    uint32_t hv = hsp_hash(str);
    uint16_t id = hsp_find(hv);
    if (id) {
        const char* str2 = hsp.table[id];
        if (strcmp(str, str2) != 0) {
            log_error("found hash (0x%08X) collision \"%s\"  vs  \"%s\"", hv, str, str2);
            log_info("%d strings in pool", hsp.last + 1);
            EK_ASSERT(0);
        }
        return hv;
    }
    uint16_t new_id = ++hsp.last;
    const uint32_t max_length = sizeof(hsp.table) / sizeof(hsp.table[0]);
    const uint32_t str_buf_size = strlen(str) + 1;
    if (str_buf_size > max_length) {
        log_error("got string length %d (maximum length is %d)", str_buf_size, max_length);
        EK_ASSERT(0);
    }
    strcpy(hsp.table[new_id], str);
    hsp_insert(hv, new_id);
    return hv;
}

const char* hsp_get(string_hash_t hv) {
    const uint16_t id = hsp_find(hv);
    // table[0] is NULL
    return id ? hsp.table[id] : "";
}

#else

const char* hsp_get(string_hash_t hv) {
    UNUSED(hv);
    return "";
}

#endif