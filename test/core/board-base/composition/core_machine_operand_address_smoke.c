#include "lib/types/file.h"
#include "../core_machine_board_fixture.h"
#include "lib/types/types_interface.h"
#include "core/x86/device_support_interface.h"
#include "core/board-base/machine_board_interface.h"

#define OAS_GDT_POINTER 0x0100u
#define OAS_GDT_ADDRESS 0x0300u
#define OAS_CODE_ADDRESS 0x2000u
#define OAS_DATA_ADDRESS 0x3000u

typedef struct oas_machine {
    core_machine *machine;
} oas_machine;

typedef struct oas_port_state {
    lib_u32 reads;
    lib_u32 writes;
    lib_u32 last_write;
} oas_port_state;

static lib_status oas_port_read(void *owner, lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    oas_port_state *state = (oas_port_state *)owner;

    if (state == LIB_NULL || out_value == LIB_NULL || port != 0x00e0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++state->reads;
    *out_value = 0x5au;
    return LIB_STATUS_OK;
}

static lib_status oas_port_write(void *owner, lib_u16 port, lib_u32 value)
{
    oas_port_state *state = (oas_port_state *)owner;

    if (state == LIB_NULL || port != 0x00e0u || value > 0xffu)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++state->writes;
    state->last_write = value;
    return LIB_STATUS_OK;
}

static const core_machine_port_provider oas_port_provider = {
    oas_port_read, oas_port_write
};

static lib_i32 oas_write(oas_machine *state, lib_u32 address,
    const void *bytes, lib_size byte_count)
{
    return core_machine_memory_write(state->machine, address,
        bytes, byte_count) == LIB_STATUS_OK;
}

static lib_i32 oas_patch(oas_machine *state,
    core_machine_debug_register register_id, lib_u32 value)
{
    core_machine_debug_register_patch patch = {0};

    patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(register_id);
    patch.values[register_id] = value;
    return core_machine_debug_patch_registers(state->machine, &patch) ==
        LIB_STATUS_OK;
}

static lib_i32 oas_capture(oas_machine *state,
    core_machine_debug_cpu_snapshot *out_cpu)
{
    return core_machine_debug_capture_cpu_snapshot(state->machine,
        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, out_cpu) == LIB_STATUS_OK;
}

static lib_i32 oas_prepare(oas_machine *state, oas_port_state *port,
    lib_bool limited, lib_bool expand_down)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    const lib_u8 gdt_pointer[] = { 0x1fu,0,0,0x03u,0,0 };
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0xcfu,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0x40u,0
    };
    const lib_u8 real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    core_machine_run_result result = {0};

    lib_memory_set(state, 0, sizeof(*state));
    if (limited) {
        gdt[16u] = 0x10u;
        gdt[17u] = 0u;
        gdt[21u] = expand_down ? 0x96u : 0x92u;
        gdt[22u] = expand_down ? 0u : 0x40u;
    }
    if (core_machine_create(&config, &state->machine, LIB_NULL) != LIB_STATUS_OK ||
        (port != LIB_NULL && core_machine_install_port_provider(state->machine,
            0x00e0u, 0x00e0u, &oas_port_provider, port) != LIB_STATUS_OK) ||
        core_machine_freeze_execution_providers(state->machine) != LIB_STATUS_OK ||
        core_machine_reset(state->machine) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(state->machine, &entry) !=
            LIB_STATUS_OK ||
        !oas_write(state, OAS_GDT_POINTER, gdt_pointer, sizeof(gdt_pointer)) ||
        !oas_write(state, OAS_GDT_ADDRESS, gdt, sizeof(gdt)) ||
        !oas_write(state, 0u, real_code, sizeof(real_code))) {
        lib_c_fprintf(lib_c_stderr, "operand address board bootstrap setup failed\n");
        return 0;
    }
    if (core_machine_run(state->machine,
            (core_machine_run_budget){ 10u, 0u }, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 10u) {
        lib_c_fprintf(lib_c_stderr, "operand address board bootstrap run failed reason=%u executed=%u\n",
            (unsigned)result.reason, (unsigned)result.executed);
        return 0;
    }
    return 1;
}

static lib_i32 oas_run_halt(oas_machine *state, const lib_u8 *code,
    lib_size code_size, core_machine_debug_cpu_snapshot *out_cpu)
{
    core_machine_run_result result = {0};

    return oas_write(state, OAS_CODE_ADDRESS, code, code_size) &&
        core_machine_run(state->machine,
            (core_machine_run_budget){ 48u, 0u }, &result) == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT &&
        oas_capture(state, out_cpu);
}

static lib_i32 oas_run_gp(oas_machine *state, const lib_u8 *code,
    lib_size code_size, core_machine_debug_cpu_snapshot *out_cpu)
{
    core_machine_run_result result = {0};
    lib_status status;

    if (!oas_write(state, OAS_CODE_ADDRESS, code, code_size)) return 0;
    status = core_machine_run(state->machine,
        (core_machine_run_budget){ 16u, 0u }, &result);
    if (!oas_capture(state, out_cpu)) return 0;
    if (!test_core_machine_fixture_shutdown_wait(status, &result)) {
        lib_c_fprintf(lib_c_stderr, "operand address shutdown status=%u reason=%u detail=%08x eip=%08x\n",
            (unsigned)status, (unsigned)result.reason, (unsigned)result.detail,
            (unsigned)out_cpu->eip);
        return 0;
    }
    return 1;
}

static lib_i32 oas_test_fault_delivery(void)
{
    static const lib_u8 invalid_data[] = {
        0x8bu,0x05u,0x20u,0x00u,0x00u,0x00u
    };
    static const lib_u8 invalid_expand_down[] = {
        0x8bu,0x05u,0x10u,0x00u,0x00u,0x00u
    };
    const lib_u8 *codes[] = { invalid_data, invalid_expand_down };
    oas_machine state;
    lib_u8 form;

    for (form = 0u; form != 2u; ++form) {
        core_machine_debug_cpu_snapshot after = {0};
        lib_i32 failed = !oas_prepare(&state, LIB_NULL, LIB_TRUE,
            form == 1u);

        if (!failed) {
            failed |= !oas_patch(&state, CORE_MACHINE_DEBUG_EAX,
                form == 0u ? 0xaabbccddu : 0x11223344u) ||
                !oas_run_gp(&state, codes[form], sizeof(invalid_data),
                    &after) ||
                after.eax != (form == 0u ? 0xaabbccddu : 0x11223344u);
        }
        core_machine_destroy(state.machine);
        if (failed) {
            lib_c_fprintf(lib_c_stderr, "operand address fault form=%u failed\n",
                (unsigned)form);
            return 0;
        }
    }
    return 1;
}

static lib_i32 oas_test_io_strings(void)
{
    static const lib_u8 outsb[] = { 0x66u,0xbau,0xe0u,0,0xbeu,0,1,0,0,
        0xb9u,2,0,0,0,0xf3u,0x6eu,0xf4u };
    static const lib_u8 insb[] = { 0x66u,0xbau,0xe0u,0,0xbfu,0,1,0,0,
        0xb9u,2,0,0,0,0xf3u,0x6cu,0xf4u };
    static const lib_u8 source[] = { 0x31u,0x42u };
    lib_u8 destination[2] = {0};
    oas_port_state port = {0};
    oas_machine state;
    core_machine_debug_cpu_snapshot after = {0};
    lib_i32 failed = !oas_prepare(&state, &port, LIB_FALSE, LIB_FALSE);

    if (!failed) {
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0100u,
            source, sizeof(source)) ||
            !oas_run_halt(&state, outsb, sizeof(outsb), &after) ||
            port.reads || port.writes != 2u || port.last_write != 0x42u ||
            after.esi != 0x0102u || after.ecx != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    lib_memory_set(&port, 0, sizeof(port));
    failed = !oas_prepare(&state, &port, LIB_FALSE, LIB_FALSE);
    if (!failed) {
        failed |= !oas_run_halt(&state, insb, sizeof(insb), &after) ||
            port.reads != 2u || port.writes || after.edi != 0x0102u ||
            after.ecx != 0u ||
            core_machine_memory_read(state.machine,
                OAS_DATA_ADDRESS + 0x0100u, destination,
                sizeof(destination)) != LIB_STATUS_OK ||
            destination[0] != 0x5au || destination[1] != 0x5au;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!oas_test_fault_delivery()) {
        lib_c_fprintf(lib_c_stderr, "operand address fault delivery failed\n");
        return 1;
    }
    if (!oas_test_io_strings()) {
        lib_c_fprintf(lib_c_stderr, "operand address I/O strings failed\n");
        return 1;
    }
    lib_c_printf("M5::OPERAND-ADDRESS-STACK:OK\n");
    return 0;
}
