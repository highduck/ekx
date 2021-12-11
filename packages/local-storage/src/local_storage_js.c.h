extern bool web_ls_set_f64(const char* key, double value);
extern bool web_ls_get_f64(const char* key, double* dest);
extern bool web_ls_set(const char* key, const char* value);
extern bool web_ls_get(const char* key, char* dest, int maxSize, uint32_t* out_bytes_written);

void ek_ls_set_i(const char* key, int value) {
    EK_ASSERT(key != 0);
    web_ls_set_f64(key, (double)value);
}

int ek_ls_get_i(const char* key, int de_fault) {
    EK_ASSERT(key != 0);
    double v;
    if(web_ls_get_f64(key, &v)) {
        // possibly need correction
        return (int)v;
    }
    return de_fault;
}

void ek_ls_set_s(const char* key, const char* value) {
    EK_ASSERT(key != 0);
    web_ls_set(key, value);
}

int ek_ls_get_s(const char* key, char* buffer, uint32_t buffer_size) {
    EK_ASSERT(key != 0);
    EK_ASSERT(buffer != 0);

    uint32_t written;
    if(web_ls_get(key, buffer, buffer_size, &written)) {
        return written;
    }
    return 0;
}
