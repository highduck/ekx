#ifndef EK_RR_H
#define EK_RR_H

#include <stdint.h>
#include <ek/hash.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t res_id;

#define REF_TO(x) res_id

// array of data and names indexed by `static id`
// array is only glowing up
typedef struct rr_man_t {
    // data vector (slots)
    string_hash_t* names; // [data_max]
    void* data; // [data_max]

    // control structure
    uint16_t max;
    uint16_t num;
    uint32_t data_size;
} rr_man_t;

res_id rr_named(rr_man_t* man, string_hash_t name);

#ifndef NDEBUG

rr_man_t* rr_verify_man(rr_man_t* man);
res_id rr_verify_id(rr_man_t* man, res_id res);

// get data by static id (this version checks manager state, name collision, index out-of-bounds)
#define REF_RESOLVE(man, id) ((man).data[rr_verify_id(&((man).rr), (id))])

#else

/// without checking
#define REF_RESOLVE(man, id) ((man).data[(id)])

#endif

// get static id by "global name hash"
#define REF_NAME(man, name) (rr_named(&(man).rr, (name)))

// get data by "global name hash"
#define RES_NAME_RESOLVE(man, name) REF_RESOLVE(man, REF_NAME(man, name))


#ifdef __cplusplus
}
#endif

#endif // EK_RR_H
