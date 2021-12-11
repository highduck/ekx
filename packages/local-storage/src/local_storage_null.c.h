#define C_STRING_NOT_NULL_OR_EMPTY(x) ((x) != 0 && *(x) != 0)
#define ASSERT_KEY_IS_VALID(x) EKAPP_ASSERT(C_STRING_NOT_NULL_OR_EMPTY(x))

void ek_ls_set_i(const char* key, int value) {
    (void)value;
    ASSERT_KEY_IS_VALID(key);
}

int ek_ls_get_i(const char* key, int de_fault) {
    ASSERT_KEY_IS_VALID(key);
    return de_fault;
}

void ek_ls_set_s(const char* key, const char* str) {
    (void)str;
    ASSERT_KEY_IS_VALID(key);
}

int ek_ls_get_s(const char* key, char* buffer, uint32_t buffer_size) {
    ASSERT_KEY_IS_VALID(key);
    return 0;
}
