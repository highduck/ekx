#include "ref.h"
#include "assert.h"

#ifdef __cplusplus
extern "C" {
#endif

void ek_ref_table_init(ek_ref_table* table, const char* type, ek_ref_finalizer finalizer) {
    EK_ASSERT(table);
    table->type = type;
    table->finalizer = finalizer;
    table->count = 1;
}

// return existing data index by name
// if not found creates new data handle, associate it with name, reset data to default state from data[0]
ek_ref_t ek_ref_make(ek_ref_table* table, const char* name, const char* type) {
    EK_ASSERT(table);
    EK_ASSERT(!type || type == table->type);
    EK_ASSERT(name);
    EK_ASSERT(strlen(name) < EK_REF_NAME_MAX);

    // start from #1 item
    const int next = table->count;
#pragma nounroll
    for (int i = 1; i < next; ++i) {
        if (strcmp(table->names[i].data, name) == 0) {
            return (ek_ref_t) {(uint16_t) (i & 0xFFFFu)};
        }
    }

    if (next == EK_REF_MAX_COUNT) {
        return (ek_ref_t) {.id=0};
    }
    ++table->count;
    ek_ref_item* item = table->items + next;
    item = table->items + next;
    strcpy(table->names[next].data, name);
    //memset(ek_image_reg_.data + slot_index, 0, sizeof(ek_image_reg_.data[0]));
    return (ek_ref_t) {(uint16_t) (next & 0xFFFFu)};
}

ek_ref_item* ek_ref_get_item(ek_ref_table* table, ek_ref_t ref, const char* type) {
    EK_ASSERT(table);
    EK_ASSERT(!type || type == table->type);
    EK_ASSERT(ref.id != 0);
    EK_ASSERT(ref.id < table->count);
    return table->items + ref.id;
}

void ek_ref_replace(ek_ref_table* table, ek_ref_t ref, ek_ref_item item_data, const char* type) {
    EK_ASSERT(table);
    EK_ASSERT(!type || type == table->type);
    EK_ASSERT(ref.id != 0);
    EK_ASSERT(ref.id < table->count);

    ek_ref_item* item = table->items + ref.id;
    if (table->finalizer) {
        table->finalizer(item);
    }
    *item = item_data;
}

#ifdef __cplusplus
}
#endif
