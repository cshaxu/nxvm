#include "lib/types/types_interface.h"
#include <stdio.h>
#include "x86/chips/cpu/cpu.h"
#include "x86/core/debug_interface.h"
#include "x86/core/machine_interface.h"
#include "support/cpu_board_fault_fixture.h"
#include "support/cpu_board_de_fixture.h"

static lib_i32 inc_dec_first_group_protected_faults(void)
{
    static const test_cpu_board_fault_case cases[] = {
        {{0xffu,0x06u,0x10u,0u},4u,LIB_TRUE, LIB_TRUE, 0x7fffu,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF},
        {{0xffu,0x06u,0x10u,0u},4u,LIB_FALSE,LIB_FALSE,0x7fffu,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x16u,0x10u,0u},4u,LIB_TRUE, LIB_TRUE, 0x55aau,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x16u,0x10u,0u},4u,LIB_FALSE,LIB_FALSE,0x55aau,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x1eu,0x10u,0u},4u,LIB_TRUE, LIB_TRUE, 0x7fffu,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x1eu,0x10u,0u},4u,LIB_FALSE,LIB_FALSE,0x7fffu,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x06u,0x10u,0u,0xffu,0xffu},6u,LIB_TRUE,LIB_TRUE,
            0xffffu,0u,0u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF},
        {{0xf7u,0x26u,0x10u,0u},4u,LIB_TRUE,LIB_TRUE,2u,
            0xaabb8000u,0x11223344u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x2eu,0x10u,0u},4u,LIB_TRUE,LIB_TRUE,2u,
            0xaabb8000u,0x11223344u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_ZF},
        {{0xf7u,0x36u,0x10u,0u},4u,LIB_TRUE,LIB_TRUE,2u,
            5u,0xaabbccddu,VCPU_EFLAGS_CF | VCPU_EFLAGS_OF},
        {{0xf7u,0x3eu,0x10u,0u},4u,LIB_TRUE,LIB_TRUE,2u,
            5u,0xaabbccddu,VCPU_EFLAGS_CF | VCPU_EFLAGS_OF}
    };

    return test_cpu_board_faults(cases,
        sizeof(cases) / sizeof(cases[0]));
}

static lib_i32 inc_dec_first_group_divide_delivery(void)
{
    static const lib_u8 code[][3] = {
        {0xf6u,0xf1u}, {0xf6u,0xf9u}, {0xf7u,0xf1u}, {0xf7u,0xf9u},
        {0x66u,0xf7u,0xf1u}, {0x66u,0xf7u,0xf9u}
    };

    for (lib_u8 fault_case = 0u; fault_case != 2u; ++fault_case)
    for (lib_u8 form = 0u; form != sizeof(code) / sizeof(code[0]); ++form) {
        const lib_i32 signed_divide = (form & 1u) != 0u;
        const lib_u8 bytes = form < 2u ? 1u : (form < 4u ? 2u : 4u);
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const lib_u32 eax = fault_case ? (signed_divide ?
            (bytes == 1u ? 0xff80u : (bytes == 2u ?
            0x00008000u : 0x80000000u)) : 0u) : 5u;
        const test_cpu_board_de_case entry = {
            .code = {code[form][0], code[form][1], code[form][2]},
            .bytes = form < 4u ? 2u : 3u,
            .eax = fault_case && !signed_divide && bytes == 1u ?
                0x00000100u : eax,
            .edx = fault_case ? (bytes == 1u ? 0x11223344u :
                (signed_divide ? (bytes == 2u ? 0x0000ffffu :
                0xffffffffu) : 1u)) : 0xaabbccddu,
            .ecx = fault_case ? (signed_divide ? mask : 1u) : 0u,
            .flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF
        };
        if (!test_cpu_board_de_delivery(&entry)) {
            fprintf(stderr, "S33 divide delivery case %u/%u failed\n",
                (unsigned)fault_case, (unsigned)form);
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    if (!inc_dec_first_group_protected_faults() ||
        !inc_dec_first_group_divide_delivery()) {
        fputs("M5:T539:S33:BOARD-INC-DEC-GROUP:FAIL\n", stderr);
        return 1;
    }
    puts("M5:T539:S33:BOARD-INC-DEC-GROUP:OK");
    return 0;
}
