#ifndef EK_FIREBASE_H
#define EK_FIREBASE_H

#ifdef __cplusplus
extern "C" {
#endif

void ek_firebase_init(void);

void ek_firebase_screen(const char* name);

void ek_firebase_event(const char* action, const char* target);

#ifdef __cplusplus
}
#endif

#endif // EK_FIREBASE_H
