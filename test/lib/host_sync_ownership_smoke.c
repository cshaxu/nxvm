#include "lib/types/types_interface.h"
#include "lib/types/win32/sync.h"
#include "lib/host/sync_interface.h"
#include <assert.h>

static unsigned allocations, releases, joins;
static void *allocate(lib_size count, lib_size size)
{ ++allocations; return lib_allocate_zero(count, size); }
static void release(void *memory)
{ if (memory) ++releases; lib_release(memory); }
static DWORD WINAPI join_wait(HANDLE handle, DWORD timeout)
{ ++joins; return WaitForSingleObject(handle, timeout); }
#undef lib_allocate_zero
#undef lib_release
#undef lib_win32_wait_for_single_object
#define lib_allocate_zero allocate
#define lib_release release
#define lib_win32_wait_for_single_object join_wait
#include "lib/host/win32/sync.c"

static void task_entry(void *context, const host_sync_task *task)
{
    unsigned *completed = context;
    assert(host_sync_task_wait_cancel(task, LIB_UINT32_MAX) == HOST_SYNC_WAIT_CANCELLED);
    *completed = 1;
}

int main(void)
{
    host_sync_event *event;
    host_sync_task *task;
    unsigned completed = 0;
    assert(host_sync_event_create(&event) == LIB_STATUS_OK);
    assert(allocations == 1);
    host_sync_event_signal(event);
    assert(host_sync_event_wait(event, 0) == HOST_SYNC_WAIT_SIGNALED);
    host_sync_event_reset(event);
    assert(host_sync_event_wait(event, 0) == HOST_SYNC_WAIT_TIMED_OUT);
    host_sync_event_destroy(event);
    assert(releases == 1);
    assert(host_sync_task_create(task_entry, &completed, &task) == LIB_STATUS_OK);
    assert(allocations == 3); /* cancellation event + thread, no startup allocation */
    host_sync_task_destroy(task);
    assert(completed && joins == 1 && allocations == releases);
    return 0;
}
