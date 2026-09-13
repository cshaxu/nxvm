#ifndef LIB_CONSOLE_MUTEX_H
#define LIB_CONSOLE_MUTEX_H
#include "lib/types/types_interface.h"

/* Component-private blocking gates. Callbacks must not reenter a gate. */
typedef struct console_mutex console_mutex;
lib_status console_mutex_create(console_mutex **out_mutex);
void console_mutex_destroy(console_mutex *mutex);
void console_mutex_enter(console_mutex *mutex);
void console_mutex_leave(console_mutex *mutex);
#endif
