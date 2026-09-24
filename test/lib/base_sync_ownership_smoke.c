#include "lib/types/types_interface.h"
#include "lib/types/win32/sync.h"
#include "lib/base/sync_interface.h"
#include <assert.h>

static lib_u32 allocations, releases, joins, allocation_attempts, fail_allocation;
static lib_i32 fail_create, fail_signal, fail_reset, fail_thread, fail_join;
static lib_iptr observed_identity;
static HANDLE WINAPI create_event(LPSECURITY_ATTRIBUTES attributes, BOOL manual,
    BOOL initial, LPCSTR name)
{ return fail_create ? NULL : CreateEventA(attributes, manual, initial, name); }
static BOOL WINAPI signal_event(HANDLE handle)
{ return fail_signal ? FALSE : SetEvent(handle); }
static BOOL WINAPI reset_event(HANDLE handle)
{ return fail_reset ? FALSE : ResetEvent(handle); }
static void *allocate(lib_size count, lib_size size)
{
    if (++allocation_attempts == fail_allocation) return NULL;
    ++allocations; return lib_allocate_zero(count, size);
}
static void release(void *memory)
{ if (memory) ++releases; lib_release(memory); }
static DWORD WINAPI join_wait(HANDLE handle, DWORD timeout)
{ ++joins; return fail_join ? WAIT_FAILED : WaitForSingleObject(handle, timeout); }
static HANDLE WINAPI create_thread(LPSECURITY_ATTRIBUTES a, SIZE_T size,
    LPTHREAD_START_ROUTINE entry, LPVOID context, DWORD flags, LPDWORD id)
{ return fail_thread ? NULL : CreateThread(a, size, entry, context, flags, id); }
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
    assert(base_sync_task_wait_cancel(task, LIB_UINT32_MAX) == BASE_SYNC_WAIT_CANCELLED);
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
    assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &event) == LIB_STATUS_OK);
    assert(allocations == 1);
    assert(base_sync_event_signal(event) == LIB_STATUS_OK);
    assert(base_sync_event_wait(event, 0) == BASE_SYNC_WAIT_SIGNALED);
    assert(base_sync_event_wait(event, 0) == BASE_SYNC_WAIT_SIGNALED);
    assert(base_sync_event_reset(event) == LIB_STATUS_OK);
    assert(base_sync_event_wait(event, 0) == BASE_SYNC_WAIT_TIMED_OUT);
    base_sync_event_destroy(event);
    assert(releases == 1);
    assert(base_sync_task_create(task_entry, &completed, &task) == LIB_STATUS_OK);
    assert(allocations == 3); /* cancellation event + thread, no startup allocation */
    identity = (lib_iptr)task;
    assert(base_sync_task_destroy(task) == LIB_STATUS_OK);
    assert(observed_identity == identity && completed && joins == 1 && allocations == releases);
    /* Count both common and platform allocation sites, including failed create. */
    for (lib_u32 failure = 1; failure <= 2; ++failure) {
        fail_allocation = allocation_attempts + failure;
        task = (void *)1;
        assert(base_sync_task_create(task_entry, &completed, &task) == LIB_STATUS_NO_MEMORY);
        assert(task == NULL && allocations == releases);
    }
    fail_allocation = 0;
    fail_thread = 1;
    assert(base_sync_task_create(task_entry, &completed, &task) == LIB_STATUS_IO_ERROR);
    assert(task == NULL && allocations == releases && joins == 1);
    fail_thread = 0; fail_create = 1;
    assert(base_sync_task_create(task_entry, &completed, &task) == LIB_STATUS_IO_ERROR);
    assert(task == NULL && allocations == releases);
    fail_create = 0;
    assert(base_sync_task_create(NULL, NULL, &task) == LIB_STATUS_INVALID_ARGUMENT && !task);
    assert(base_sync_task_create(task_entry, NULL, NULL) == LIB_STATUS_INVALID_ARGUMENT);
    assert(base_sync_task_destroy(NULL) == LIB_STATUS_OK);
    assert(base_sync_task_join(NULL) == LIB_STATUS_OK);
    assert(base_sync_task_request_cancel(NULL) == LIB_STATUS_OK);
    assert(base_sync_task_create(immediate_entry, &completed, &task) == LIB_STATUS_OK);
    identity = (lib_iptr)task;
    assert(base_sync_task_destroy(task) == LIB_STATUS_OK);
    assert(observed_identity == identity && completed == 2 && joins == 2 && allocations == releases);
    assert(base_sync_task_create(task_entry, &completed, &task) == LIB_STATUS_OK);
    {
        lib_u32 before = releases;
        fail_signal = 1;
        assert(base_sync_task_destroy(task) == LIB_STATUS_IO_ERROR);
        assert(releases == before && joins == 2);
        fail_signal = 0; fail_join = 1;
        assert(base_sync_task_destroy(task) == LIB_STATUS_IO_ERROR);
        assert(releases == before && joins == 3);
        assert(task->cancellation != NULL); /* No reference has been disposed. */
        fail_join = 0;
        assert(base_sync_task_destroy(task) == LIB_STATUS_OK);
        assert(allocations == releases && joins == 4);
    }
    event = (void *)1;
    assert(base_sync_event_create((base_sync_event_mode)99, &event) ==
        LIB_STATUS_INVALID_ARGUMENT && event == NULL);
    assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, NULL) ==
        LIB_STATUS_INVALID_ARGUMENT);
    fail_create = 1;
    assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &event) ==
        LIB_STATUS_IO_ERROR && event == NULL);
    fail_create = 0;
    assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &event) == LIB_STATUS_OK);
    assert(base_sync_event_signal(NULL) == LIB_STATUS_INVALID_ARGUMENT);
    assert(base_sync_event_reset(NULL) == LIB_STATUS_INVALID_ARGUMENT);
    assert(base_sync_event_wait(NULL, 0) == BASE_SYNC_WAIT_INVALID_ARGUMENT);
    assert(base_sync_event_signal(event) == LIB_STATUS_OK);
    assert(base_sync_event_signal(event) == LIB_STATUS_OK);
    assert(base_sync_event_wait(event, 0) == BASE_SYNC_WAIT_SIGNALED);
    assert(base_sync_event_wait(event, 0) == BASE_SYNC_WAIT_TIMED_OUT);
    fail_signal = fail_reset = 1;
    assert(base_sync_event_signal(event) == LIB_STATUS_IO_ERROR);
    assert(base_sync_event_reset(event) == LIB_STATUS_IO_ERROR);
    fail_signal = fail_reset = 0;
    assert(base_sync_event_signal(event) == LIB_STATUS_OK);
    assert(base_sync_event_reset(event) == LIB_STATUS_OK);
    assert(base_sync_event_wait(event, 0) == BASE_SYNC_WAIT_TIMED_OUT);
    base_sync_event_destroy(event);
    assert(allocations == releases);
    return 0;
}
