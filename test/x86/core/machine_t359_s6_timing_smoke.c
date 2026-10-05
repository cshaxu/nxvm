#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86/core/device_support_interface.h"

#include "x86/core/machine_interface.h"
#include "x86/core/debug_interface.h"
#include "exception_fixture.h"

#define T359_S6_RESET_LINEAR 0xfffffff0u
#define T359_S6_RESET_PHYSICAL 0x000ffff0u
#define T359_S6_CODE_LINEAR 0u
#define T359_S6_DATA 0x00001000u

typedef struct t359_s6_state {
    lib_u64 advanced_ticks;
} t359_s6_state;

typedef struct t359_s6_row {
    const lib_u8 *program;
    lib_size program_bytes;
    lib_u64 ticks;
    lib_u32 eax;
} t359_s6_row;

static void t359_s6_reset(void *opaque)
{
    t359_s6_state *state = (t359_s6_state *)opaque;

    if (state != LIB_NULL) state->advanced_ticks = 0u;
}

static void t359_s6_advance(void *opaque, lib_u64 ticks)
{
    t359_s6_state *state = (t359_s6_state *)opaque;

    if (state != LIB_NULL) state->advanced_ticks += ticks;
}

static const core_machine_execution_provider t359_s6_execution = {
    t359_s6_reset, t359_s6_advance
};

static lib_i32 t359_s6_enter_protected(core_machine *machine,
    lib_u64 *out_setup_ticks)
{
    static const lib_u8 gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0u,0u,0x9au,0u,0u,
        0xffu,0xffu,0u,0u,0u,0x92u,0u,0u
    };
    static const lib_u8 pointer[] = {0x17u, 0u, 0u, 0x03u, 0u, 0u};
    static const lib_u8 setup[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u, /* lgdt [100h] */
        0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u, 0xf0u, /* lmsw ax */
        0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xd0u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = {[CORE_MACHINE_DEBUG_EIP] = 0x0200u}
    };
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};

    if (core_machine_memory_write(machine, 0x0300u, gdt, sizeof(gdt)) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0100u, pointer, sizeof(pointer)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0200u, setup, sizeof(setup)) != LIB_STATUS_OK ||
        core_machine_run(machine, (core_machine_run_budget){7u, 0u}, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 7u ||
        core_machine_debug_capture_cpu_snapshot(machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT,
            &snapshot) != LIB_STATUS_OK ||
        (snapshot.cr0 & VCPU_CR0_PE) == 0u || snapshot.cs.selector != 8u ||
        snapshot.cs.base != 0u || snapshot.cs.limit != 0xffffu || snapshot.cs.dpl != 0u ||
        snapshot.ds.selector != 0x10u || snapshot.ds.base != 0u ||
        snapshot.ds.limit != 0xffffu || snapshot.ds.dpl != 0u ||
        snapshot.ss.selector != 0x10u || snapshot.ss.base != 0u ||
        snapshot.ss.limit != 0xffffu || snapshot.ss.dpl != 0u ||
        snapshot.gdtr.base != 0x0300u || snapshot.gdtr.limit != 0x17u ||
        snapshot.eip != 0u)
    {
        lib_c_fprintf(lib_c_stderr, "S6 setup failed: executed=%llu reason=%d cr0=%x cs=%x base=%x ss=%x ip=%x\n",
            result.executed, result.reason, snapshot.cr0, snapshot.cs.selector,
            snapshot.cs.base, snapshot.ss.selector, snapshot.eip);
        return 0;
    }
    *out_setup_ticks = result.elapsed_ticks;
    return 1;
}

static lib_i32 t359_s6_prepare(core_machine **out_machine, t359_s6_state *state)
{
    const core_machine_executor_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_memory_alias_config aliases[] = {
        {T359_S6_RESET_LINEAR, T359_S6_RESET_PHYSICAL, 16u},
        {T359_S6_CODE_LINEAR, T359_S6_CODE_LINEAR, 64u},
        {T359_S6_DATA, T359_S6_DATA, 64u}
    };
    core_machine *machine = LIB_NULL;

    if (out_machine == LIB_NULL || state == LIB_NULL ||
        core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_install_memory_aliases(machine, aliases,
            sizeof(aliases) / sizeof(aliases[0]), LIB_FALSE) != LIB_STATUS_OK ||
        test_core_exception_block_vector(machine, 6u, state) != LIB_STATUS_OK ||
        core_machine_bind_execution_provider(machine, &t359_s6_execution, state) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 t359_s6_run(core_machine *machine, t359_s6_state *state,
    const t359_s6_row *row)
{
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    lib_u64 data = UINT64_C(0x8877665544332211);
    lib_status status;
    lib_u64 setup_ticks;

    if (machine == LIB_NULL || state == LIB_NULL || row == LIB_NULL ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, T359_S6_CODE_LINEAR, row->program,
            row->program_bytes) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, T359_S6_DATA, &data, sizeof(data)) !=
            LIB_STATUS_OK) return 0;
    if (!t359_s6_enter_protected(machine, &setup_ticks) ||
        state->advanced_ticks != setup_ticks ||
        core_machine_debug_write_register(machine, CORE_MACHINE_DEBUG_EAX,
            row->eax) != LIB_STATUS_OK) return 0;
    status = core_machine_run(machine, budget, &result);
    return status == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 1u &&
        result.ticks == row->ticks && result.elapsed_ticks == setup_ticks + row->ticks &&
        state->advanced_ticks == setup_ticks + row->ticks;
}

static lib_i32 t359_s6_test_fixed_real_rows(void)
{
    static const lib_u8 clts[] = { 0x0fu, 0x06u };
    static const lib_u8 mov_from_cr0[] = { 0x0fu, 0x20u, 0xc0u };
    static const lib_u8 mov_to_cr0[] = { 0x0fu, 0x22u, 0xc0u };
    static const lib_u8 mov_from_tr6[] = { 0x0fu, 0x24u, 0xf0u };
    static const lib_u8 mov_to_tr6[] = { 0x0fu, 0x26u, 0xf0u };
    static const lib_u8 sgdt[] = {
        0x0fu, 0x01u, 0x06u, 0x00u, 0x10u
    };
    static const lib_u8 smsw_register[] = { 0x0fu, 0x01u, 0xe0u };
    static const lib_u8 smsw_memory[] = {
        0x0fu, 0x01u, 0x26u, 0x00u, 0x10u
    };
    static const lib_u8 lmsw_register[] = { 0x0fu, 0x01u, 0xf0u };
    static const lib_u8 push_fs[] = { 0x0fu, 0xa0u };
    static const t359_s6_row rows[] = {
        { clts, sizeof(clts), 6u, 0u },
        { mov_from_cr0, sizeof(mov_from_cr0), 6u, 0u },
        { mov_to_cr0, sizeof(mov_to_cr0), 11u, 0u },
        { mov_from_tr6, sizeof(mov_from_tr6), 12u, 0u },
        { mov_to_tr6, sizeof(mov_to_tr6), 12u, 0u },
        { sgdt, sizeof(sgdt), 9u, 0u },
        { smsw_register, sizeof(smsw_register), 2u, 0u },
        { smsw_memory, sizeof(smsw_memory), 2u, 0u },
        { lmsw_register, sizeof(lmsw_register), 11u, 0u },
        { push_fs, sizeof(push_fs), 2u, 0u }
    };
    t359_s6_state state = { 0u };
    core_machine *machine = LIB_NULL;
    lib_size index;
    lib_i32 failed = !t359_s6_prepare(&machine, &state);

    for (index = 0u; !failed && index < sizeof(rows) / sizeof(rows[0]); ++index) {
        if (!t359_s6_run(machine, &state, &rows[index])) {
            lib_c_fprintf(lib_c_stderr, "S6 privileged timing row %u failed\n", (lib_u32)index);
            failed = 1;
        }
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 t359_s6_test_rejected_lock(void)
{
    static const lib_u8 locked_clts[] = { 0xf0u, 0x0fu, 0x06u };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    t359_s6_state state = { 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !t359_s6_prepare(&machine, &state);

    /* T337_REAL_UD_TERMINAL_IVT_REJECT: the vector route fails without retirement. */
    if (!failed && core_machine_reset(machine) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, T359_S6_RESET_LINEAR, locked_clts,
            sizeof(locked_clts)) == LIB_STATUS_OK) {
        failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.executed != 0u || result.ticks != 0u ||
            state.advanced_ticks != 0u;
    } else {
        failed = 1;
    }
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    if (t359_s6_test_fixed_real_rows() || t359_s6_test_rejected_lock()) return 1;
    lib_c_printf("M5:T359:S6:PRIVILEGED-TIMING:OK\n");
    return 0;
}
