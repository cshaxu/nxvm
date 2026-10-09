#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "core/chips/cpu/cpu_interface.h"
#include "core/x86/debug_interface.h"
#include "debug_fixture.h"

#define CALL_GATE_GDT_POINTER 0x0100u
#define CALL_GATE_GDT_BASE 0x0300u
#define CALL_GATE_TSS_BASE 0x0600u
#define CALL_GATE_KERNEL_BASE 0x2000u
#define CALL_GATE_USER_CODE_BASE 0x4000u
#define CALL_GATE_USER_DATA_BASE 0x5000u

typedef struct call_gate_machine {
    core_machine *machine;
} call_gate_machine;

static lib_i32 call_gate_prepare(call_gate_machine *state)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_neutral_create(&config, &state->machine) != LIB_STATUS_OK) return 0;
    if (core_machine_freeze_execution_providers(state->machine) != LIB_STATUS_OK ||
        core_machine_reset(state->machine) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(state->machine,
            &(core_machine_debug_register_patch){
                .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
                    CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
            }) != LIB_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 call_gate_write(call_gate_machine *machine, lib_u32 address,
    const lib_u8 *bytes, lib_size count)
{
    return core_machine_memory_write(machine->machine, address, bytes, count) ==
        LIB_STATUS_OK;
}

static lib_i32 call_gate_install(call_gate_machine *state)
{
    static const lib_u8 gdt_pointer[] = {
        0x37u,0x00u,0x00u,0x03u,0x00u,0x00u
    };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0xff,0xff,0,0x40,0,0xfa,0,0,
        0xff,0xff,0,0x50,0,0xf2,0,0,
        0x2b,0,0,0x06,0,0x81,0,0,
        0x00,0x01,0x08,0x00,0x00,0xe4,0x00,0x00
    };
    static const lib_u8 real_code[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x28,0x00,0x0f,0x00,0xd8,
        0xb8,0x10,0x00,0x8e,0xd0,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    static const lib_u8 kernel_code[] = {
        0xb8,0x10,0x00,0x8e,0xd8,
        0xb8,0x23,0x00,0x50,
        0xb8,0x00,0xa0,0x50,
        0xb8,0x02,0x02,0x50,
        0xb8,0x1b,0x00,0x50,
        0xb8,0x00,0x00,0x50,
        0xb8,0x23,0x00,0x8e,0xd8,
        0xcf
    };
    static const lib_u8 gate_target[] = {
        0xb8,0x11,0x11,0xa3,0x00,0x00,0xcb
    };
    static const lib_u8 user_code[] = {
        0x9a,0x00,0x00,0x33,0x00,
        0xb8,0x22,0x22,0xa3,0x02,0x00,0xeb,0xfe
    };
    const lib_u16 sp0 = 0x9000u;
    const lib_u16 ss0 = 0x0010u;

    return call_gate_write(state, CALL_GATE_GDT_POINTER, gdt_pointer,
            sizeof(gdt_pointer)) &&
        call_gate_write(state, CALL_GATE_GDT_BASE, gdt, sizeof(gdt)) &&
        call_gate_write(state, CALL_GATE_TSS_BASE + 2u, (const lib_u8 *)&sp0,
            sizeof(sp0)) &&
        call_gate_write(state, CALL_GATE_TSS_BASE + 4u, (const lib_u8 *)&ss0,
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
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    lib_u16 markers[2] = {0u, 0u};
    const core_machine_run_budget budget = { 1024u, 0u };
    lib_i32 failed = !call_gate_prepare(&state);

    if (!failed) {
        failed |= !call_gate_install(&state);
        failed |= core_machine_run(state.machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET;
        failed |= core_machine_memory_read(state.machine, CALL_GATE_USER_DATA_BASE,
            markers, sizeof(markers)) != LIB_STATUS_OK || markers[0] != 0x1111u ||
            markers[1] != 0x2222u;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK || diagnostic.first_fault.valid ||
            diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 0u;
        if (failed) {
            core_machine_debug_cpu_snapshot snapshot = {0};
            (void)core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &snapshot);
            lib_c_fprintf(lib_c_stderr,
                "call-gate result=%u markers=%04x/%04x fault=%d mask=%08x code=%08x pc=%04x:%08x cs=%04x sp=%04x\n",
                (unsigned)result.reason, markers[0], markers[1],
                diagnostic.first_fault.valid, diagnostic.first_fault.exception_mask,
                diagnostic.first_fault.exception_code,
                diagnostic.first_fault.point.cs, diagnostic.first_fault.point.eip,
                snapshot.cs.selector,
                (lib_u16)snapshot.esp);
        }
    }
    core_machine_destroy(state.machine);
    if (failed) return 1;
    lib_c_printf("CALL-GATE-16:OK\n");
    return 0;
}
