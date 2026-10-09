#include "lib/types/test.h"
#include "emulator/machine/input_queue.h"


int main(void)
{
    emulator_machine_input_queue storage = { 0 }, *queue = &storage;
    kvm_input_event first = { 0 };
    kvm_input_event second = { 0 };
    kvm_input_event actual = { 0 };

    first.type = KVM_EVENT_MOUSE;
    first.data.mouse.relative = LIB_TRUE;
    first.data.mouse.delta_y = 16;
    first.data.mouse.buttons = KVM_MOUSE_BUTTON_LEFT;
    second = first;
    second.data.mouse.delta_y = 16;
    lib_test_assert(emulator_machine_input_queue_initialize(queue) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_input_queue_push(queue, &first));
    lib_test_assert(emulator_machine_input_queue_push(queue, &second));
    lib_test_assert(emulator_machine_input_queue_pop(queue, &actual));
    lib_test_assert(lib_memory_compare(&actual, &first, sizeof(actual)) == 0);
    lib_test_assert(emulator_machine_input_queue_pop(queue, &actual));
    lib_test_assert(lib_memory_compare(&actual, &second, sizeof(actual)) == 0);
    lib_test_assert(!emulator_machine_input_queue_pending(queue));
    emulator_machine_input_queue_dispose(queue);
    return 0;
}
