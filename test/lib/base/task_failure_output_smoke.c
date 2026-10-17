#include "lib/types/test.h"
#include "lib/base/sync_interface.h"
int main(void)
{
    base_sync_task *task = (base_sync_task *)1;

    lib_test_assert(base_sync_mutex_create(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    base_sync_mutex_destroy(LIB_NULL);
    lib_test_assert(base_sync_event_signal(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);

    lib_test_assert(base_sync_task_create(LIB_NULL, LIB_NULL, &task) ==
        LIB_STATUS_INVALID_ARGUMENT && task == LIB_NULL);
    return 0;
}
