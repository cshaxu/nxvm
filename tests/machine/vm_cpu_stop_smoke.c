#include <stdio.h>

#include "core/machine/cpu.h"
#include "core/machine/memory.h"
#include "vm/composition_control.h"

int main(void)
{
    vm_composition_live_machine session = {0};
    const unsigned char invalid_instruction[] = { 0x0fu, 0x0bu };
    unsigned char *reset_vector;
    int failed = 0;

    vm_composition_live_machine_initialize(&session);
    vm_composition_control_initialize(session.control, &session);
    vm_composition_control_reset(session.control);
    reset_vector = (unsigned char *)core_machine_memory_real_address(
        session.ram, 0xf000u, 0xfff0u);
    reset_vector[0] = invalid_instruction[0];
    reset_vector[1] = invalid_instruction[1];
    vm_composition_control_start(session.control);

    failed |= vm_composition_control_is_running(session.control) != False;
    failed |= core_machine_cpu_execution_consume_stop_request(
        session.cpu_execution) != False;
    vm_composition_control_finalize(session.control, &session);
    vm_composition_live_machine_finalize(&session);

    if (failed) return 1;
    puts("M5:T14:S3:CPU-STOP:OK");
    return 0;
}
