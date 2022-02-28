#ifndef EK_FIREBASE_H
#define EK_FIREBASE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum firebase_cmd_t {
    FIREBASE_CMD_INIT = 0,
    FIREBASE_CMD_AUTH_LOGIN = 1,
    FIREBASE_CMD_AUTH_LOGOUT = 2,
    FIREBASE_CMD_AUTH_DELETE_ACCOUNT = 3,
} firebase_cmd_t;

bool firebase(firebase_cmd_t cmd);

void firebase_screen(const char* name);

void firebase_event(const char* action, const char* target);

#ifdef __cplusplus
}
#endif

#endif // EK_FIREBASE_H
