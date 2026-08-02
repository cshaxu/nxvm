#include <stdio.h>

#include "core/machine/cpu.h"
#include "core/machine/vram.h"
#include "vm/composition_control.h"

int main(void)
{
    const unsigned char invalid_instruction[] = { 0x0fu, 0x0bu };
    unsigned char *reset_vector;
    int failed = 0;

    vm_composition_control_initialize();
    vm_composition_control_reset();
    reset_vector = (unsigned char *)vramGetRealAddr(0xf000u, 0xfff0u);
    reset_vector[0] = invalid_instruction[0];
    reset_vector[1] = invalid_instruction[1];
    vm_composition_control_start();

    failed |= vm_composition_control_is_running() != False;
    failed |= vcpuConsumeStopRequest() != False;
    vm_composition_control_finalize();

    if (failed) return 1;
    puts("M5:T14:S3:CPU-STOP:OK");
    return 0;
}
