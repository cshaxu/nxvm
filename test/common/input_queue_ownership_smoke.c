#include "lib/types/test.h"
#include "lib/types/file.h"
#include "common/machine/input_queue.h"

static lib_bool fail_mutex;
static lib_status create_mutex(base_sync_mutex **out)
{
    if (!fail_mutex) return base_sync_mutex_create(out);
    *out = LIB_NULL;
    return LIB_STATUS_NO_MEMORY;
}
#define base_sync_mutex_create create_mutex
#include "common/machine/input_queue.c"

int main(void)
{
    common_machine_input_queue queue = { 0 };
    kvm_input_event input = { 0 }, output;
    lib_u32 i;
    lib_test_assert(common_machine_input_queue_initialize(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    fail_mutex = LIB_TRUE;
    lib_test_assert(common_machine_input_queue_initialize(&queue) == LIB_STATUS_NO_MEMORY);
    common_machine_input_queue_dispose(&queue);
    fail_mutex = LIB_FALSE;
    lib_test_assert(common_machine_input_queue_initialize(&queue) == LIB_STATUS_OK);
    for (i = 0; i < 600u; ++i) {
        input.source_identity = i;
        lib_test_assert(common_machine_input_queue_push(&queue, &input));
        lib_test_assert(common_machine_input_queue_pop(&queue, &output));
        lib_test_assert(output.source_identity == i);
    }
    common_machine_input_queue_dispose(&queue);
    lib_test_assert(queue.lock == LIB_NULL);
    return 0;
}
