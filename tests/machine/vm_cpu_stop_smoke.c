#include <stdio.h>

#include "core/machine/vcpu.h"
#include "core/machine/vram.h"
#include "vm/machine/device.h"

int main(void)
{
    const unsigned char invalid_instruction[] = { 0x0fu, 0x0bu };
    unsigned char *reset_vector;
    int failed = 0;

    deviceInit();
    deviceReset();
    reset_vector = (unsigned char *)vramGetRealAddr(0xf000u, 0xfff0u);
    reset_vector[0] = invalid_instruction[0];
    reset_vector[1] = invalid_instruction[1];
    deviceStart();

    failed |= device.flagRun != False;
    failed |= vcpuConsumeStopRequest() != False;
    deviceFinal();

    if (failed) return 1;
    puts("M5:T14:S3:CPU-STOP:OK");
    return 0;
}
