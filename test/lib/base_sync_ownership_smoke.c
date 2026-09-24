#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "lib/types/win32/sync.h"
#include "lib/base/sync_interface.h"

static lib_u32 allocations, releases, joins, allocation_attempts, fail_allocation;
static lib_i32 fail_create, fail_signal, fail_reset, fail_thread, fail_join;
static lib_iptr observed_identity;
static lib_win32_handle LIB_WIN32_WINAPI create_event(lib_win32_lpsecurity_attributes attributes, lib_win32_bool manual,
    lib_win32_bool initial, lib_win32_lpcstr name)
{ return fail_create ? LIB_NULL : lib_win32_create_event_a(attributes, manual, initial, name); }
static lib_win32_bool LIB_WIN32_WINAPI signal_event(lib_win32_handle handle)
{ return fail_signal ? LIB_WIN32_FALSE : lib_win32_set_event(handle); }
static lib_win32_bool LIB_WIN32_WINAPI reset_event(lib_win32_handle handle)
{ return fail_reset ? LIB_WIN32_FALSE : lib_win32_reset_event(handle); }
static void *allocate(lib_size count, lib_size size)
{
    if (++allocation_attempts == fail_allocation) return LIB_NULL;
    ++allocations; return lib_allocate_zero(count, size);
}
static void release(void *memory)
{ if (memory) ++releases; lib_release(memory); }
static lib_win32_dword LIB_WIN32_WINAPI join_wait(lib_win32_handle handle, lib_win32_dword timeout)
{ ++joins; return fail_join ? LIB_WIN32_WAIT_FAILED : lib_win32_wait_for_single_object(handle, timeout); }
static lib_win32_handle LIB_WIN32_WINAPI create_thread(lib_win32_lpsecurity_attributes a, lib_win32_size_t size,
    lib_win32_thread_start_routine entry, lib_win32_lpvoid context, lib_win32_dword flags, lib_win32_lpdword id)
{ return fail_thread ? LIB_NULL : lib_win32_create_thread(a, size, entry, context, flags, id); }
#undef lib_allocate_zero
#undef lib_release
#undef lib_win32_wait_for_single_object
#define lib_allocate_zero allocate
#define lib_release release
#define lib_win32_wait_for_single_object join_wait
#undef lib_win32_create_event_a
#undef lib_win32_set_event
#undef lib_win32_reset_event
#define lib_win32_create_event_a create_event
#define lib_win32_set_event signal_event
#define lib_win32_reset_event reset_event
#undef lib_win32_create_thread
#define lib_win32_create_thread create_thread
#include "lib/base/win32/sync.c"
#include "lib/base/sync.c"

static void task_entry(void *context, const base_sync_task *task)
{
    lib_u32 *completed = context;
    observed_identity = (lib_iptr)task;
    lib_test_assert(base_sync_task_wait_cancel(task, LIB_UINT32_MAX) == BASE_SYNC_WAIT_CANCELLED);
    *completed = 1;
}
static void immediate_entry(void *context, const base_sync_task *task)
{
    observed_identity = (lib_iptr)task;
    *(lib_u32 *)context = 2;
}

int main(void)
{
    base_sync_event *event;
    base_sync_task *task;
    lib_iptr identity;
    lib_u32 completed = 0;
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &event) == LIB_STATUS_OK);
    lib_test_assert(allocations == 1);
    lib_test_assert(base_sync_event_signal(event) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(event, 0) == BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(base_sync_event_wait(event, 0) == BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(base_sync_event_reset(event) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(event, 0) == BASE_SYNC_WAIT_TIMED_OUT);
    base_sync_event_destroy(event);
    lib_test_assert(releases == 1);
    lib_test_assert(base_sync_task_create(task_entry, &completed, &task) == LIB_STATUS_OK);
    lib_test_assert(allocations == 3); /* cancellation event + thread, no startup allocation */
    identity = (lib_iptr)task;
    lib_test_assert(base_sync_task_destroy(task) == LIB_STATUS_OK);
    lib_test_assert(observed_identity == identity && completed && joins == 1 && allocations == releases);
    /* Count both common and platform allocation sites, including failed create. */
    for (lib_u32 failure = 1; failure <= 2; ++failure) {
        fail_allocation = allocation_attempts + failure;
        task = (void *)1;
        lib_test_assert(base_sync_task_create(task_entry, &completed, &task) == LIB_STATUS_NO_MEMORY);
        lib_test_assert(task == LIB_NULL && allocations == releases);
    }
    fail_allocation = 0;
    fail_thread = 1;
    lib_test_assert(base_sync_task_create(task_entry, &completed, &task) == LIB_STATUS_IO_ERROR);
    lib_test_assert(task == LIB_NULL && allocations == releases && joins == 1);
    fail_thread = 0; fail_create = 1;
    lib_test_assert(base_sync_task_create(task_entry, &completed, &task) == LIB_STATUS_IO_ERROR);
    lib_test_assert(task == LIB_NULL && allocations == releases);
    fail_create = 0;
    lib_test_assert(base_sync_task_create(LIB_NULL, LIB_NULL, &task) == LIB_STATUS_INVALID_ARGUMENT && !task);
    lib_test_assert(base_sync_task_create(task_entry, LIB_NULL, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(base_sync_task_destroy(LIB_NULL) == LIB_STATUS_OK);
    lib_test_assert(base_sync_task_join(LIB_NULL) == LIB_STATUS_OK);
    lib_test_assert(base_sync_task_request_cancel(LIB_NULL) == LIB_STATUS_OK);
    lib_test_assert(base_sync_task_create(immediate_entry, &completed, &task) == LIB_STATUS_OK);
    identity = (lib_iptr)task;
    lib_test_assert(base_sync_task_destroy(task) == LIB_STATUS_OK);
    lib_test_assert(observed_identity == identity && completed == 2 && joins == 2 && allocations == releases);
    lib_test_assert(base_sync_task_create(task_entry, &completed, &task) == LIB_STATUS_OK);
    {
        lib_u32 before = releases;
        fail_signal = 1;
        lib_test_assert(base_sync_task_destroy(task) == LIB_STATUS_IO_ERROR);
        lib_test_assert(releases == before && joins == 2);
        fail_signal = 0; fail_join = 1;
        lib_test_assert(base_sync_task_destroy(task) == LIB_STATUS_IO_ERROR);
        lib_test_assert(releases == before && joins == 3);
        lib_test_assert(task->cancellation != LIB_NULL); /* No reference has been disposed. */
        fail_join = 0;
        lib_test_assert(base_sync_task_destroy(task) == LIB_STATUS_OK);
        lib_test_assert(allocations == releases && joins == 4);
    }
    event = (void *)1;
    lib_test_assert(base_sync_event_create((base_sync_event_mode)99, &event) ==
        LIB_STATUS_INVALID_ARGUMENT && event == LIB_NULL);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, LIB_NULL) ==
        LIB_STATUS_INVALID_ARGUMENT);
    fail_create = 1;
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &event) ==
        LIB_STATUS_IO_ERROR && event == LIB_NULL);
    fail_create = 0;
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &event) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_signal(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(base_sync_event_reset(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(base_sync_event_wait(LIB_NULL, 0) == BASE_SYNC_WAIT_INVALID_ARGUMENT);
    lib_test_assert(base_sync_event_signal(event) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_signal(event) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(event, 0) == BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(base_sync_event_wait(event, 0) == BASE_SYNC_WAIT_TIMED_OUT);
    fail_signal = fail_reset = 1;
    lib_test_assert(base_sync_event_signal(event) == LIB_STATUS_IO_ERROR);
    lib_test_assert(base_sync_event_reset(event) == LIB_STATUS_IO_ERROR);
    fail_signal = fail_reset = 0;
    lib_test_assert(base_sync_event_signal(event) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_reset(event) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(event, 0) == BASE_SYNC_WAIT_TIMED_OUT);
    base_sync_event_destroy(event);
    lib_test_assert(allocations == releases);
    return 0;
}
