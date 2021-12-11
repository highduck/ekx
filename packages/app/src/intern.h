#ifndef EK_APP_INTERN_H
#define EK_APP_INTERN_H

#include <ek/app.h>

#ifdef __cplusplus
extern "C" {
#endif

void ek_app__init(void);

void ek_app__process_event(ek_app_event event);

void ek_app__process_frame(void);

void ek_app__notify_ready(void);

void ek_app__update_viewport(ek_app_viewport viewport);

#ifdef __cplusplus
}
#endif

#endif // EK_APP_INTERN_H
