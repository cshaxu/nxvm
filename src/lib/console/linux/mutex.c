#include "lib/console/mutex.h"
#include "lib/types/linux/sync.h"

struct console_mutex { lib_linux_pthread_mutex_t gate; };
lib_status console_mutex_create(console_mutex **out_mutex)
{
    console_mutex *mutex = lib_allocate(sizeof(*mutex));
    *out_mutex = LIB_NULL;
    if (mutex == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (lib_linux_pthread_mutex_init(&mutex->gate, LIB_NULL) != 0) {
        lib_release(mutex);
        return LIB_STATUS_IO_ERROR;
    }
    *out_mutex = mutex;
    return LIB_STATUS_OK;
}
void console_mutex_destroy(console_mutex *mutex)
{
    if (mutex == LIB_NULL) return;
    (void)lib_linux_pthread_mutex_destroy(&mutex->gate);
    lib_release(mutex);
}
void console_mutex_enter(console_mutex *mutex)
{ (void)lib_linux_pthread_mutex_lock(&mutex->gate); }
void console_mutex_leave(console_mutex *mutex)
{ (void)lib_linux_pthread_mutex_unlock(&mutex->gate); }
