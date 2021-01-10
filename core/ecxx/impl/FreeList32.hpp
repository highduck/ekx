#pragma once

#include <stdint.h>

#define GENERATION_MASK 0xFF00000
#define INDEX_MASK 0x00FFFFF;
#define INDEX_BITS 20;

// Element is Free when `element index` != `value index` (value & INDEX_MASK)
// 0 element is preserved for invalid/nothing
struct freelist_t {
    uint32_t* elements;
    uint32_t size;
    uint32_t next;
};

void freelist_reset(freelist_t* list) {
    list->elements[0] = 0;
    for (uint32_t i = 1; i < list->size; ++i) {
        list->elements[i] = i + 1;
    }
    list->next = 1;
}

void freelist_init(freelist_t* list, uint32_t capacity) {
    list->elements = new uint32_t[capacity];
    list->size = capacity;
    freelist_init(list);
}

void freelist_destroy(freelist_t* list) {
    delete[] list->elements;
    *list = {};
}

void freelist_allocate(freelist_t* list, const uint32_t count, uint32_t* out) {
    uint32_t* elements = list->elements;
    uint32_t next = list->next;
    for (uint32_t i = 0; i < count; ++i) {
        // write
        *(out++) = next;
        next = elements[next];
    }
    list->next = next;
}

uint32_t freelist_allocate(freelist_t* list) {
    uint32_t id;
    freelist_allocate_n(list, 1, &id);
    return id;
}

void freelist_deallocate(freelist_t* list, const uint32_t* ids, const uint32_t count) {
    uint32_t* elements = list->elements;
    uint32_t next = list->next;
    for (uint32_t i = 0; i < count; ++i) {
        const index = ids[i];
        elements[index] = next;
        next = index;
    }
    list->next = next;
}

void freelist_deallocate(freelist_t* list, uint32_t id) {
    freelist_deallocate(list, &id, 1);
}

uint32_t freelist_allocate_generation(freelist_t* list, const uint32_t indexMask, const uint32_t generationMask) {
    // read
    uint32_t* elements = list->elements;
    const uint32_t nextFreeIndex = list->next;
    const uint32_t nextFreeValue = elements[nextFreeIndex];
    const uint32_t gid = nextFreeIndex | (nextFreeValue & generationMask);

    // write back
    elements[next] = gid;
    list->next = nextFreeValue & indexMask;

    return gid;
}

void freelist_allocate_generation_n(freelist_t* list, const uint32_t count, uint32_t* out,
                                    const uint32_t indexMask,
                                    const uint32_t generationMask) {
    for (uint32_t i = 0; i < count; ++i) {
        *(out++) = freelist_allocate_generation(list, indexMask, generationMask);
    }
}

// nextGenerationBits = ((++generation) << INDEX_BITS)
void freelist_deallocate_generation_pair(freelist_t* list, uint32_t index, uint32_t nextGenerationBits) {
    list->elements[index] = list->next | nextGenerationBits;
    list->next = index;
}

void freelist_deallocate_generation(freelist_t* list, uint32_t id, const uint32_t generationIncBits) {
    const uint32_t indexMask = generationIncBits - 1;
    const uint32_t generationMask = ~indexMask;
    const uint32_t index = id & indexMask;
    //deallocate(id & indexMask, (id + generationIncBits) & generationMask);
    list->elements[index] = list->next | ((id + generationIncBits) & generationMask);
    list->next = index;
}