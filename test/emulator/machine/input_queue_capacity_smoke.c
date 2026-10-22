#include "lib/types/test.h"
#include "emulator/machine/input_queue.h"
int main(void)
{
    emulator_machine_input_queue storage = {0}, *input = &storage;
    kvm_input_event key = {0}, copied = {0};
    lib_u32 i;
    lib_test_assert(emulator_machine_input_queue_initialize(input) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_input_queue_push(input, &key) == LIB_STATUS_INVALID_STATE);
    emulator_machine_input_queue_admit(input, LIB_TRUE);
    key.type = KVM_EVENT_KEY;
    for (i = 0; i < 255u; ++i) {
        key.source_identity = i;
        lib_test_assert(emulator_machine_input_queue_push(input, &key) == LIB_STATUS_OK);
    }
    lib_test_assert(emulator_machine_input_queue_push(input, &key) == LIB_STATUS_LIMIT_EXCEEDED);
    emulator_machine_input_queue_admit(input, LIB_FALSE);
    lib_test_assert(emulator_machine_input_queue_push(input, &key) == LIB_STATUS_INVALID_STATE);
    for (i = 0; i < 255u; ++i) {
        lib_test_assert(emulator_machine_input_queue_pop(input, &copied));
        lib_test_assert(copied.source_identity == i);
    }
    lib_test_assert(!emulator_machine_input_queue_pending(input));
    emulator_machine_input_queue_admit(input, LIB_TRUE);
    lib_test_assert(emulator_machine_input_queue_push(input, &key) == LIB_STATUS_OK);
    emulator_machine_input_queue_clear(input);
    lib_test_assert(!emulator_machine_input_queue_pop(input, &copied));
    emulator_machine_input_queue_dispose(input);

    return 0;
}
