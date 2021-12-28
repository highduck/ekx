#include "temp_res_man.h"
#include "ek/log.h"
#include <ek/handle2.h>
#include <time.h>

typedef struct ek_res_info {
    void* data;
    char* name;
    ek_handle2_state handles;
    uint32_t data_stride;
    uint32_t name_max;

    void (* destroy_fn)(void* data);
} ek_res_info;

void ek_res_create(ek_res_info* res, int num, int data_stride, int name_max) {
    const uint32_t data_size = num * data_stride;
    const uint32_t name_size = num * name_max;
    const uint32_t dense_size = num * 4;
    const uint32_t sparse_size = num * 2;
    char* data = calloc(1, data_size + name_size + dense_size + sparse_size);
    res->data = data;
    data += data_size;
    res->name = data;
    data += name_size;
    res->handles.dense = (uint32_t*) data;
    data += dense_size;
    res->handles.sparse = (uint16_t*) data;
    data += sparse_size;
    res->name_max = name_max;
    res->data_stride = data_stride;
    res->destroy_fn = NULL;
    ek_handle2_init(res->handles, num);
}

// return existing data index by name
// if not found creates new data handle, associate it with name, reset data to default state from data[0]
uint32_t ek_res_for_name(ek_res_info* res, const char* name) {
    EK_ASSERT(res);
    EK_ASSERT(name);
    int count = ek_handle2_count(res->handles);
    const uint32_t name_max = res->name_max;
    EK_ASSERT(strlen(name) < res->name_max);
//    EK_WARN("search is slow %s", name);
//    nanosleep(&(struct timespec) {
//        .tv_sec = 0,
//        .tv_nsec = 16 * 1000000
//    }, NULL);

    char* cur_name = res->name + name_max;
#pragma nounroll
    for (int i = 1; i < count; ++i) {
        if (strcmp(cur_name, name) == 0) {
            return res->handles.dense[i];
        }
        cur_name += name_max;
    }

    const uint32_t new_id = ek_handle2_new(res->handles);
    const uint16_t slot_index = res->handles.sparse[new_id & 0xFFFFu];
    strcpy(res->name + (slot_index * name_max), name);
    //memset(ek_image_reg_.data + slot_index, 0, sizeof(ek_image_reg_.data[0]));
    return new_id;
}

void* ek_res_data(ek_res_info* res, uint32_t id) {
    EK_ASSERT(id != 0);
    EK_ASSERT(ek_handle2_valid(res->handles, id));
    const uint16_t slot_index = res->handles.sparse[id & 0xFFFF];
    return res->data + res->data_stride * slot_index;
}

void ek_res_assign(ek_res_info* res, uint32_t id, void* data) {
    void* curr = ek_res_data(res, id);
    if (res->destroy_fn) {
        res->destroy_fn(curr);
    }
    memcpy(curr, data, res->data_stride);
}

ek_res_info ek_res_images;
ek_res_info ek_res_shaders;

void ek_image__destroy(void* data) {
    sg_image* p = data;
    if (p && p->id) {
        sg_destroy_image(*p);
        p->id = 0;
    }
}

void ek_shader__destroy(void* data) {
    ek_shader* p = data;
    if (p && p->shader.id) {
        sg_destroy_shader(p->shader);
        p->shader.id = 0;
        p->images_num = 0;
    }
}

void ek_gfx_res_setup(void) {
    ek_res_create(&ek_res_images, 32, sizeof(sg_image), 32);
    ek_res_images.destroy_fn = ek_image__destroy;

    ek_res_create(&ek_res_shaders, 32, sizeof(ek_shader), 32);
    ek_res_shaders.destroy_fn = ek_shader__destroy;
}

ek_image_reg_id ek_image_reg_named(const char* name) {
    return (ek_image_reg_id) {.id = ek_res_for_name(&ek_res_images, name)};
}

void ek_image_reg_assign(ek_image_reg_id id, sg_image image) {
    ek_res_assign(&ek_res_images, id.id, &image);
}

sg_image ek_image_reg_get(ek_image_reg_id id) {
    return *((sg_image*) ek_res_data(&ek_res_images, id.id));
}

ek_shader_ref ek_shader_named(const char* name) {
    return (ek_shader_ref) {.id = ek_res_for_name(&ek_res_shaders, name)};
}

void ek_shader_register(ek_shader_ref ref, ek_shader shader) {
    ek_res_assign(&ek_res_shaders, ref.id, &shader);
}

ek_shader ek_shader_get(ek_shader_ref ref) {
    return *((ek_shader*) ek_res_data(&ek_res_shaders, ref.id));
}