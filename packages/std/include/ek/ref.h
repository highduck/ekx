#ifndef EK_REF_H
#define EK_REF_H

#include <stdint.h>

#define EK_REF_NAME_MAX (32)
#define EK_REF_MAX_COUNT (256)

typedef struct ek_ref_t {
    uint16_t id;
} ek_ref_t;

#define ek_ref(T) T##_REF
#define ek_ref_declare(T) typedef struct T##_REF {uint16_t id;} T##_REF; \
extern ek_ref_table T##_REF_TABLE; \
void T##_refs_init(void); \
void T##_REF_DESTROY(T *content);

#define ek_ref_implement(T) \
ek_ref_table T##_REF_TABLE; \
void T##_refs_init(void) {                                     \
    ek_ref_table_init(ek_ref_get_table_for_type(T), #T, (ek_ref_finalizer)T##_REF_DESTROY);           \
}

typedef struct ek_ref_name {
    char data[EK_REF_NAME_MAX];
} ek_ref_name;

typedef struct ek_ref_item {
    uint32_t handle;
    uint32_t meta;
} ek_ref_item;

typedef void (* ek_ref_finalizer)(void* item);

typedef struct ek_ref_table {
    const char* type;
    ek_ref_finalizer finalizer;
    int count;
    ek_ref_name names[EK_REF_MAX_COUNT];
    ek_ref_item items[EK_REF_MAX_COUNT];
} ek_ref_table;

#ifdef __cplusplus
extern "C" {
#endif

void ek_ref_table_init(ek_ref_table* table, const char* type, ek_ref_finalizer finalizer);

// return existing data index by name
// if not found creates new data handle, associate it with name, reset data to default state from data[0]
ek_ref_t ek_ref_make(ek_ref_table* table, const char* name, const char* type);

ek_ref_item* ek_ref_get_item(ek_ref_table* table, ek_ref_t ref, const char* type);

void ek_ref_replace(ek_ref_table* table, ek_ref_t ref, ek_ref_item item_data, const char* type);

#ifdef __cplusplus
}
#endif

#define ek_ref_get_table_for_type(T)  (&T##_REF_TABLE)
#define ek_ref_for_name(T, name) (ek_ref_make(ek_ref_get_table_for_type(T), name, #T))
#define ek_ref_cast_to_base(T, ref) ((ek_ref_t){((ek_ref(T))(ref)).id})
#define ek_ref_find(T, name) ( (ek_ref(T)){ek_ref_make(ek_ref_get_table_for_type(T), name, #T).id})
#define ek_ref_content(T, ref) (*((T*)ek_ref_get_item(ek_ref_get_table_for_type(T), ek_ref_cast_to_base(T, ref), #T)))
#define ek_ref_set(T, ref, data) do{ek_ref_replace(ek_ref_get_table_for_type(T), ref, *(ek_ref_item*)&(data), #T);} while(0)

#define ek_ref_reset(T, ref) do{ek_ref_replace(ek_ref_get_table_for_type(T),ek_ref_cast_to_base(T, ref), (ek_ref_item){}, #T);} while(0)
#define ek_ref_reset_s(T, name) do{ek_ref_replace(ek_ref_get_table_for_type(T),ek_ref_for_name(T, name), (ek_ref_item){}, #T);} while(0)
#define ek_ref_assign(T, ref, data) do{ek_ref_replace(ek_ref_get_table_for_type(T),ek_ref_cast_to_base(T, ref), *(ek_ref_item*)&(data), #T);} while(0)
#define ek_ref_assign_s(T, name, data) do{ek_ref_replace(ek_ref_get_table_for_type(T),ek_ref_for_name(T, name), *(ek_ref_item*)&(data), #T);} while(0)

#endif // EK_REF_H
