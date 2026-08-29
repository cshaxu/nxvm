#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define CALL_GATE_GDT_POINTER 0x0100u
#define CALL_GATE_GDT_BASE 0x0300u
#define CALL_GATE_TSS_BASE 0x0600u
#define CALL_GATE_KERNEL_BASE 0x2000u
#define CALL_GATE_USER_CODE_BASE 0x4000u
#define CALL_GATE_USER_DATA_BASE 0x5000u

typedef struct call_gate_machine {
    core_machine *machine;
} call_gate_machine;

static C_VOID call_gate_reset(C_VOID *opaque)
{
    call_gate_machine *state = (call_gate_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider call_gate_provider = {
    call_gate_reset, STD_NULL
};

static C_INT call_gate_prepare(call_gate_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK) return 0;
    if (!test_core_machine_fixture_bind_freeze_reset(state->machine,
            &call_gate_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT call_gate_write(call_gate_machine *machine, type_unsigned_32 address,
    const type_unsigned_8 *bytes, STD_SIZE_T count)
{
    return core_machine_memory_write(machine->machine, address, bytes, count) ==
        TYPE_STATUS_OK;
}

static C_INT call_gate_install(call_gate_machine *state)
{
    static const type_unsigned_8 gdt_pointer[] = {
        0x37u,0x00u,0x00u,0x03u,0x00u,0x00u
    };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0xff,0xff,0,0x40,0,0xfa,0,0,
        0xff,0xff,0,0x50,0,0xf2,0,0,
        0x2b,0,0,0x06,0,0x81,0,0,
        0x00,0x01,0x08,0x00,0x00,0xe4,0x00,0x00
    };
    static const type_unsigned_8 real_code[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x28,0x00,0x0f,0x00,0xd8,
        0xb8,0x10,0x00,0x8e,0xd0,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    static const type_unsigned_8 kernel_code[] = {
        0xb8,0x10,0x00,0x8e,0xd8,
        0xb8,0x23,0x00,0x50,
        0xb8,0x00,0xa0,0x50,
        0xb8,0x02,0x02,0x50,
        0xb8,0x1b,0x00,0x50,
        0xb8,0x00,0x00,0x50,
        0xb8,0x23,0x00,0x8e,0xd8,
        0xcf
    };
    static const type_unsigned_8 gate_target[] = {
        0xb8,0x11,0x11,0xa3,0x00,0x00,0xcb
    };
    static const type_unsigned_8 user_code[] = {
        0x9a,0x00,0x00,0x33,0x00,
        0xb8,0x22,0x22,0xa3,0x02,0x00,0xeb,0xfe
    };
    const type_unsigned_16 sp0 = 0x9000u;
    const type_unsigned_16 ss0 = 0x0010u;

    return call_gate_write(state, CALL_GATE_GDT_POINTER, gdt_pointer,
            sizeof(gdt_pointer)) &&
        call_gate_write(state, CALL_GATE_GDT_BASE, gdt, sizeof(gdt)) &&
        call_gate_write(state, CALL_GATE_TSS_BASE + 2u, (const type_unsigned_8 *)&sp0,
            sizeof(sp0)) &&
        call_gate_write(state, CALL_GATE_TSS_BASE + 4u, (const type_unsigned_8 *)&ss0,
            sizeof(ss0)) &&
        call_gate_write(state, 0u, real_code, sizeof(real_code)) &&
        call_gate_write(state, CALL_GATE_KERNEL_BASE, kernel_code,
            sizeof(kernel_code)) &&
        call_gate_write(state, CALL_GATE_KERNEL_BASE + 0x100u, gate_target,
            sizeof(gate_target)) &&
        call_gate_write(state, CALL_GATE_USER_CODE_BASE, user_code,
            sizeof(user_code));
}

int main(void)
{
    call_gate_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_16 markers[2] = {0u, 0u};
    const core_machine_run_budget budget = { 1024u, 0u };
    C_INT failed = !call_gate_prepare(&state);

    if (!failed) {
        failed |= !call_gate_install(&state);
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET;
        failed |= core_machine_memory_read(state.machine, CALL_GATE_USER_DATA_BASE,
            markers, sizeof(markers)) != TYPE_STATUS_OK || markers[0] != 0x1111u ||
            markers[1] != 0x2222u;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 0u;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T288 S2 call-gate result=%u markers=%04x/%04x fault=%d mask=%08x code=%08x pc=%04x:%08x cs=%04x sp=%04x\n",
                (unsigned)result.reason, markers[0], markers[1],
                diagnostic.first_fault.valid, diagnostic.first_fault.exception_mask,
                diagnostic.first_fault.exception_code,
                diagnostic.first_fault.point.cs, diagnostic.first_fault.point.eip,
                test_core_machine_fixture_capture_cpu_after_run(
                    state.machine).data.cs.selector,
                test_core_machine_fixture_capture_cpu_after_run(
                    state.machine).data.sp);
        }
    }
    core_machine_destroy(state.machine);
    if (failed) return 1;
    STD_PRINTF("M5:T288:S2:CALL-GATE-16:OK\n");
    return 0;
}
