#include "support/cpu_task_switch16_fixture.h"
#include <stdio.h>

static lib_bool cpu_task32_expect_decode(cpu_task16_case test_case,
    lib_u16 expected_saved_ip)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};
    lib_u16 saved_ip = 0u;

    cpu_task16_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386, test_case);
    cpu_task16_refresh(&fixture, 8u);
    after = fixture.cpu;
    lib_memory_copy(&saved_ip, fixture.memory + CPU_TASK16_A_BASE + 0x0eu,
        sizeof(saved_ip));
    if (!fixture.fault.valid && !fixture.delivered_exception.valid &&
        after.data.flagHalt && after.data.tr.selector == 0x30u &&
        after.data.eax == 0xffff2222u && saved_ip == expected_saved_ip)
        return LIB_TRUE;
    fprintf(stderr, "task32 decode case=%u halt=%u tr=%04x eax=%08x saved=%04x fault=%u delivered=%u\n",
        (unsigned)test_case, (unsigned)after.data.flagHalt,
        after.data.tr.selector, (unsigned)after.data.eax,
        saved_ip, (unsigned)fixture.fault.valid,
        (unsigned)fixture.delivered_exception.valid);
    return LIB_FALSE;
}

int main(void)
{
    const struct {
        cpu_task16_case test_case;
        lib_u16 expected_saved_ip;
    } cases[] = {
        {CPU_TASK16_OPERAND32, 0x000bu},
        {CPU_TASK16_INDIRECT_OPERAND32, 0x0008u},
        {CPU_TASK16_INDIRECT_ADDRESS32, 0x000au},
        {CPU_TASK16_INDIRECT_OPERAND_ADDRESS32, 0x000bu}
    };
    lib_size index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
        if (!cpu_task32_expect_decode(cases[index].test_case,
                cases[index].expected_saved_ip)) {
            fputs("M5:T539:S56:TASK32-DECODE:FAIL\n", stderr);
            return 1;
        }
    puts("M5:T539:S56:TASK32-DECODE:OK");
    return 0;
}
