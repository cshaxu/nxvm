#include "lib/types/types_interface.h"
#include <stdio.h>
#include "support/cpu_board_fault_fixture.h"

static lib_i32 inc_dec_second_group_protected_faults(void)
{
    static const test_cpu_board_fault_case cases[] = {
        {{0x84u,0x16u,0x10u,0u},4u,LIB_TRUE,LIB_TRUE,0xffffu,
            0u,0x11228000u,VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF},
        {{0x85u,0x16u,0x10u,0u},4u,LIB_TRUE,LIB_TRUE,0xffffu,
            0u,0x11228000u,VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF},
        {{0x66u,0x85u,0x16u,0x10u,0u},5u,LIB_TRUE,LIB_TRUE,0xffffu,
            0u,0x11228000u,VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF},
        {{0x01u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_TRUE,0xffffu,
            0u,1u,VCPU_EFLAGS_OF | VCPU_EFLAGS_CF},
        {{0x01u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_FALSE,0xffffu,
            0u,1u,VCPU_EFLAGS_OF | VCPU_EFLAGS_CF},
        {{0x11u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_TRUE,0xffffu,
            0u,0u,VCPU_EFLAGS_CF},
        {{0x19u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_TRUE,0u,
            0u,0u,VCPU_EFLAGS_CF},
        {{0x19u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_FALSE,0u,
            0u,0u,VCPU_EFLAGS_CF}
    };

    return test_cpu_board_faults(cases,
        sizeof(cases) / sizeof(cases[0]));
}

int main(void)
{
    if (!inc_dec_second_group_protected_faults()) {
        fputs("M5:T539:S34:BOARD-TEST-ADD-ADC-SBB:FAIL\n", stderr);
        return 1;
    }
    puts("M5:T539:S34:BOARD-TEST-ADD-ADC-SBB:OK");
    return 0;
}
