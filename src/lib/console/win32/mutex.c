#include "lib/console/mutex.h"
#include "lib/types/win32/sync.h"

struct console_mutex { lib_win32_critical_section gate; };
lib_status console_mutex_create(console_mutex **out_mutex)
{
    console_mutex *mutex = lib_allocate(sizeof(*mutex));
    *out_mutex = LIB_NULL;
    if (mutex == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    lib_win32_initialize_critical_section(&mutex->gate);
    *out_mutex = mutex;
    return LIB_STATUS_OK;
}
void console_mutex_destroy(console_mutex *mutex)
{
    if (mutex == LIB_NULL) return;
    lib_win32_delete_critical_section(&mutex->gate);
    lib_release(mutex);
}
void console_mutex_enter(console_mutex *mutex)
{ lib_win32_enter_critical_section(&mutex->gate); }
void console_mutex_leave(console_mutex *mutex)
{ lib_win32_leave_critical_section(&mutex->gate); }
