#include <ek/rr.h>
#include <ek/assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NDEBUG

uint16_t rr_verify_id(rr_man_t* man, res_id id) {
    rr_verify_man(man);
    EK_ASSERT(id > 0u);
    if(id >= man->num) {
        log_error("bad id -> %u <- is larger than current allocated number -> %u <-, pool max size: %u", id, man->num, man->max);
        EK_ASSERT(id < man->num);
    }
    EK_ASSERT(id < man->max);
    return id;
}

rr_man_t* rr_verify_man(rr_man_t* man) {
    EK_ASSERT_R2(man);
    EK_ASSERT_R2(man->data);
    EK_ASSERT(((uintptr_t) man->data % sizeof(void*)) == 0);
    EK_ASSERT(man->num > 0);
    EK_ASSERT(man->data_size > 0);
    EK_ASSERT((man->data_size % 4u) == 0);
    if (man->names) {
        EK_ASSERT(((uintptr_t) man->names % sizeof(void*)) == 0);
        EK_ASSERT(man->max > 1);
        EK_ASSERT(man->num < man->max);
    }
    return man;
}

#else // DEBUG

#define rr_check_state(man) ((void)(man))

#endif // NDEBUG

res_id rr_alloc_data_slot(rr_man_t* man) {
    // sanitize we have space for data slot
    EK_ASSERT(man->num < man->max);
    // if not debug we should not crash in case we have no more free space
    if (man->num == man->max) {
        // return null handle
        return 0u;
    }

    const res_id next = man->num++;
    // data array is always grow up and new memory is should be zeroes, so we sanitize first 4 bytes of data slot
    EK_ASSERT(*((uint32_t*) ((uint8_t*) man->data + man->data_size * next)) == 0u);

    return next;
}

res_id rr_named(rr_man_t* man, string_hash_t name) {
    EK_ASSERT(man);
    EK_ASSERT(name);
    EK_ASSERT(man->names);
    EK_ASSERT(man->max > 1);
    for (res_id i = 0; i < man->num; ++i) {
        if (man->names[i] == name) {
            return i;
        }
    }

    // create one if not found
    // sanitize we have space for name record
    EK_ASSERT(man->num < man->max);
    // if not debug we should not crash in case we have no more free space
    if (man->num == man->max) {
        // return "null handle" static id
        return 0u;
    }
    const res_id next = rr_alloc_data_slot(man);
    if (next) {
        // unused space should be zero-inited
        EK_ASSERT(man->names[next] == 0);
        man->names[next] = name;
    }
    return next;
}

#ifdef __cplusplus
}
#endif

