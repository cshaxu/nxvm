#include "type.h"

#include "lib/host/sync.h"
#include "vm/platform/execution_wait.h"

typedef struct vm_platform_execution_flip_wait {
    const lib_session_executor *executor;
    C_INT initial_flip;
} vm_platform_execution_flip_wait;

static C_INT vm_platform_execution_flip_wait_cancelled(C_VOID *context)
{
    const vm_platform_execution_flip_wait *wait = context;

    return wait == STD_NULL || lib_session_executor_get_flip(wait->executor) !=
        wait->initial_flip;
}

C_INT vm_platform_wait_for_execution_flip(const lib_session_executor *executor,
    C_INT initial_flip, type_unsigned_32 timeout_milliseconds)
{
    vm_platform_execution_flip_wait wait = { executor, initial_flip };

    if (executor == STD_NULL) return TYPE_FALSE;
    (C_VOID)host_sync_wait_milliseconds(timeout_milliseconds,
        vm_platform_execution_flip_wait_cancelled, &wait);
    return lib_session_executor_get_flip(executor) != initial_flip;
}
