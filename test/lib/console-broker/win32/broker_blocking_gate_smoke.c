#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/base/sync_interface.h"

static lib_win32_handle blocked;
static lib_win32_long contender;
static lib_u32 mutex_creates, fail_mutex, live_mutexes;
static lib_status tracked_create(base_sync_mutex **out)
{
    if (++mutex_creates == fail_mutex) {
        *out = LIB_NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    lib_status status = base_sync_mutex_create(out);
    if (status == LIB_STATUS_OK) ++live_mutexes;
    return status;
}
static void tracked_destroy(base_sync_mutex *mutex)
{
    if (mutex != LIB_NULL) { lib_test_assert(live_mutexes); --live_mutexes; }
    base_sync_mutex_destroy(mutex);
}
static void tracked_enter(base_sync_mutex *mutex)
{
    if (lib_win32_get_current_thread_id() == (lib_win32_dword)lib_win32_interlocked_compare_exchange(&contender, 0, 0)) {
        lib_win32_set_event(blocked); /* The contender has reached the owner-held lock. */
    }
    base_sync_mutex_lock(mutex);
}
#define base_sync_mutex_lock tracked_enter
#define base_sync_mutex_create tracked_create
#define base_sync_mutex_destroy tracked_destroy
#include "lib/console-broker/win32/console.c"
#undef base_sync_mutex_lock
#undef base_sync_mutex_create
#undef base_sync_mutex_destroy

static console_broker_backend backend;
static lib_i32 mode;
static lib_win32_dword LIB_WIN32_WINAPI lock_consumer(void *unused)
{
    (void)unused;
    lib_win32_interlocked_exchange(&contender, (lib_win32_long)lib_win32_get_current_thread_id());
    if (mode == 1) {
        console_broker_backend_lock_output(&backend);
        console_broker_backend_unlock_output(&backend);
    } else {
        console_broker_backend_lock_transaction(&backend);
        console_broker_backend_unlock_transaction(&backend);
    }
    return 0;
}
int main(void)
{
    lib_test_assert(base_sync_mutex_create(&backend.output_lock) == LIB_STATUS_OK);
    lib_test_assert(base_sync_mutex_create(&backend.transaction_lock) == LIB_STATUS_OK);
    for (mode = 1; mode < 3; ++mode) {
        base_sync_mutex *held = mode == 1 ? backend.output_lock : backend.transaction_lock;
        blocked = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
        lib_test_assert(blocked);
        base_sync_mutex_lock(held);
        lib_win32_handle thread = lib_win32_create_thread(LIB_NULL, 0, lock_consumer, LIB_NULL, 0, LIB_NULL);
        lib_test_assert(thread && lib_win32_wait_for_single_object(blocked, 5000) == LIB_WIN32_WAIT_OBJECT_0);
        lib_test_assert(lib_win32_wait_for_single_object(thread, 0) == LIB_WIN32_WAIT_TIMEOUT);
        base_sync_mutex_unlock(held);
        lib_test_assert(lib_win32_wait_for_single_object(thread, 5000) == LIB_WIN32_WAIT_OBJECT_0);
        lib_win32_interlocked_exchange(&contender, 0);
        lib_win32_close_handle(thread); lib_win32_close_handle(blocked);
    }
    base_sync_mutex_destroy(backend.output_lock);
    base_sync_mutex_destroy(backend.transaction_lock);
    lib_test_assert(live_mutexes == 0);
    return 0;
}
