#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "x86/chips/cpu/cpu.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "app-nxvm/devices/memory.h"
#include "support/machine_cpu_fixture.h"

#define DT_STORE_ADDRESS 0x0200u
#define DT_LOAD_ADDRESS 0x0240u
#define DT_GDT_ADDRESS 0x0300u
#define DT_LDT_SELECTOR 0x0018u
#define DT_TSS16_SELECTOR 0x0020u
#define DT_LDT_NOT_PRESENT_SELECTOR 0x0028u
#define DT_TSS16_BUSY_SELECTOR 0x0030u
#define DT_TSS16_NOT_PRESENT_SELECTOR 0x0038u
#define DT_TSS32_SELECTOR 0x0040u

typedef struct descriptor_system_machine {
    core_machine *machine;
} descriptor_system_machine;

static void dt_reset(void *opaque)
{
    descriptor_system_machine *state = (descriptor_system_machine *)opaque;

    if (state != LIB_NULL) (void)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider dt_provider = {
    dt_reset,
    LIB_NULL
};

static lib_i32 dt_prepare_profile(descriptor_system_machine *state,
    core_machine_cpu_profile profile, lib_u8 cpu_80386_cr_mov_ignores_mod)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .cpu_80386_cr_mov_ignores_mod = cpu_80386_cr_mov_ignores_mod
    };

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine, LIB_NULL) != LIB_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &dt_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 dt_prepare(descriptor_system_machine *state)
{
    return dt_prepare_profile(state, CORE_MACHINE_CPU_PROFILE_80386, LIB_FALSE);
}

static lib_i32 dt_write(descriptor_system_machine *state, lib_u32 address,
    const lib_u8 *data, lib_size bytes)
{
    return state != LIB_NULL && state->machine != LIB_NULL &&
        core_machine_memory_write(state->machine, address, data, bytes) ==
            LIB_STATUS_OK;
}

static lib_i32 dt_read(descriptor_system_machine *state, lib_u32 address,
    lib_u8 *data, lib_size bytes)
{
    return state != LIB_NULL && state->machine != LIB_NULL &&
        core_machine_memory_read(state->machine, address, data, bytes) ==
            LIB_STATUS_OK;
}


static lib_i32 dt_run(descriptor_system_machine *state, const lib_u8 *code,
    lib_size bytes, lib_i32 expect_fault, lib_u32 expect_exception)
{
    const core_machine_run_budget budget = {32u, 0u};
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;

    if (expect_fault && expect_exception == VCPUINS_EXCEPT_UD &&
        !CORE_MACHINE_BIT_IS_SET((*test_core_machine_fixture_cpu(state->machine)).data.cr0, VCPU_CR0_PE) &&
        !test_core_machine_fixture_preflight_real_ud_terminal(state->machine))
        return 0;
    if (!dt_write(state, 0u, code, bytes) ||
        core_machine_run(state->machine, budget, &result) !=
            (expect_fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
        result.reason != (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) ||
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) !=
            LIB_STATUS_OK) return 0;
    if (expect_fault && state->machine->cpu_profile >=
            CORE_MACHINE_CPU_PROFILE_80386 &&
        CORE_MACHINE_BIT_IS_SET((*test_core_machine_fixture_cpu(state->machine)).data.cr0, VCPU_CR0_PE) &&
        (expect_exception == VCPUINS_EXCEPT_TS ||
            expect_exception == VCPUINS_EXCEPT_NP ||
            expect_exception == VCPUINS_EXCEPT_SS ||
            expect_exception == VCPUINS_EXCEPT_GP)) {
        expect_exception = VCPUINS_EXCEPT_DF;
    }
    return expect_fault ? diagnostic.first_fault.valid &&
        CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask, expect_exception) :
        !diagnostic.first_fault.valid;
}

static void dt_enter_protected(descriptor_system_machine *state,
    lib_u8 cpl)
{
    t_cpu *cpu = &(*test_core_machine_fixture_cpu(state->machine));

    CORE_MACHINE_BIT_SET(cpu->data.cr0, VCPU_CR0_PE);
    cpu->data.cs.selector = (lib_u16)(0x0008u | cpl);
    cpu->data.cs.dpl = cpl;
    cpu->data.cs.base = 0u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.ds.base = 0u;
    cpu->data.ds.limit = 0xffffu;
    cpu->data.ds.selector = (lib_u16)(0x0010u | cpl);
    cpu->data.ds.flagValid = LIB_TRUE;
    cpu->data.ds.sregtype = SREG_DATA;
    cpu->data.ds.seg.executable = LIB_FALSE;
    cpu->data.ds.seg.data.writable = LIB_TRUE;
    cpu->data.ds.dpl = cpl;
}

static lib_i32 dt_test_c7_segment_override_real_mode(void)
{
    static const lib_u8 code[] = {
        0x26u, 0xc7u, 0x47u, 0x02u, 0xffu, 0xffu, 0xf4u
    };
    descriptor_system_machine state;
    lib_u16 observed = 0u;
    lib_i32 failed = !dt_prepare(&state);

    if (!failed) {
        (*test_core_machine_fixture_cpu(state.machine)).data.ebx = 0u;
        failed = !dt_run(&state, code, sizeof(code), 0, 0u) ||
            !dt_read(&state, 2u, (lib_u8 *)&observed, sizeof(observed)) ||
            observed != 0xffffu;
    }
    core_machine_destroy(state.machine);
    return !failed;
}
static lib_i32 dt_real_data_cache(const t_cpu_data_sreg *sreg,
    lib_u16 selector, t_cpu_data_sreg_type type)
{
    return sreg->flagValid && sreg->selector == selector &&
        sreg->base == (lib_u32)selector << 4u &&
        sreg->limit == 0xffffu && sreg->dpl == 0u &&
        sreg->sregtype == type && sreg->seg.accessed &&
        !sreg->seg.executable && sreg->seg.data.writable &&
        !sreg->seg.data.big && !sreg->seg.data.expdown;
}
static lib_i32 dt_test_leave_protected_mode(void)
{
    static const lib_u8 code[] = {
        0x0fu, 0x22u, 0xc0u, 0xeau, 0x0au, 0x00u, 0x00u, 0x00u,
        0x00u, 0x00u, 0xbbu, 0x48u, 0x00u, 0x8eu, 0xc3u, 0x8eu,
        0xd3u, 0x8eu, 0xdbu, 0xf4u
    };
    descriptor_system_machine state;
    lib_i32 failed = !dt_prepare(&state);

    if (!failed) {
        dt_enter_protected(&state, 0u);
        (*test_core_machine_fixture_cpu(state.machine)).data.cs.seg.exec.defsize = LIB_TRUE;
        (*test_core_machine_fixture_cpu(state.machine)).data.ds.seg.data.big = LIB_TRUE;
        (*test_core_machine_fixture_cpu(state.machine)).data.es.seg.data.big = LIB_TRUE;
        (*test_core_machine_fixture_cpu(state.machine)).data.ss.seg.data.big = LIB_TRUE;
        (*test_core_machine_fixture_cpu(state.machine)).data.eax = 0u;
        failed = !dt_run(&state, code, sizeof(code), 0, 0u) ||
            (*test_core_machine_fixture_cpu(state.machine)).data.cr0 != 0u ||
            (*test_core_machine_fixture_cpu(state.machine)).data.cs.selector != 0u ||
            (*test_core_machine_fixture_cpu(state.machine)).data.cs.base != 0u ||
            (*test_core_machine_fixture_cpu(state.machine)).data.cs.limit != 0xffffu ||
            !(*test_core_machine_fixture_cpu(state.machine)).data.cs.flagValid ||
            !(*test_core_machine_fixture_cpu(state.machine)).data.cs.seg.accessed ||
            !(*test_core_machine_fixture_cpu(state.machine)).data.cs.seg.executable ||
            (*test_core_machine_fixture_cpu(state.machine)).data.cs.seg.exec.defsize ||
            (*test_core_machine_fixture_cpu(state.machine)).data.cs.seg.exec.conform ||
            !(*test_core_machine_fixture_cpu(state.machine)).data.cs.seg.exec.readable ||
            (*test_core_machine_fixture_cpu(state.machine)).data.ebx != 0x00000048u ||
            !dt_real_data_cache(&(*test_core_machine_fixture_cpu(state.machine)).data.es, 0x0048u,
                SREG_DATA) ||
            !dt_real_data_cache(&(*test_core_machine_fixture_cpu(state.machine)).data.ss, 0x0048u,
                SREG_STACK) ||
            !dt_real_data_cache(&(*test_core_machine_fixture_cpu(state.machine)).data.ds, 0x0048u,
                SREG_DATA);
    }
    core_machine_destroy(state.machine);
    return !failed;
}
lib_i32 main(void)
{
    if (!dt_test_c7_segment_override_real_mode() ||
        !dt_test_leave_protected_mode()) return 1;
    printf("M5:T304:CONTROL-STATE:OK\n");
    return 0;
}
