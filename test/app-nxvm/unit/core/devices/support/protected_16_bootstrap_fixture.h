#ifndef TEST_PROTECTED_16_BOOTSTRAP_FIXTURE_H
#define TEST_PROTECTED_16_BOOTSTRAP_FIXTURE_H

#include "lib/types/types_interface.h"
#include "x86/chips/cpu/cpu_interface.h"
#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/devices/machine_interface.h"

#define TEST_PROTECTED_16_GDT_POINTER 0x0100u
#define TEST_PROTECTED_16_GDT_BASE 0x0300u
#define TEST_PROTECTED_16_IDT_POINTER 0x0120u
#define TEST_PROTECTED_16_IDT_BASE 0x0400u
#define TEST_PROTECTED_16_CODE_BASE 0x2000u
#define TEST_PROTECTED_16_STACK_TOP 0x8000u
#define TEST_PROTECTED_16_RESET_PHYSICAL 0x000ffff0u

#if defined(__GNUC__) || defined(__clang__)
#define TEST_PROTECTED_16_UNUSED __attribute__((unused))
#else
#define TEST_PROTECTED_16_UNUSED
#endif

typedef struct test_protected_16_machine {
    core_machine *machine;
} test_protected_16_machine;

static lib_i32 TEST_PROTECTED_16_UNUSED test_protected_16_write(test_protected_16_machine *state,
    lib_u32 address, const void *data, lib_size bytes)
{
    return state != LIB_NULL && state->machine != LIB_NULL &&
        core_machine_memory_write(state->machine, address, data, bytes) ==
            LIB_STATUS_OK;
}

static lib_i32 TEST_PROTECTED_16_UNUSED test_protected_16_read(const test_protected_16_machine *state,
    lib_u32 address, void *data, lib_size bytes)
{
    return state != LIB_NULL && state->machine != LIB_NULL &&
        core_machine_memory_read(state->machine, address, data, bytes) ==
            LIB_STATUS_OK;
}

static lib_i32 TEST_PROTECTED_16_UNUSED test_protected_16_snapshot(const test_protected_16_machine *state,
    core_machine_debug_cpu_snapshot *out_snapshot)
{
    return state != LIB_NULL && state->machine != LIB_NULL && out_snapshot != LIB_NULL &&
        core_machine_debug_capture_cpu_snapshot(state->machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, out_snapshot) == LIB_STATUS_OK;
}

static lib_i32 TEST_PROTECTED_16_UNUSED test_protected_16_install_gate(test_protected_16_machine *state,
    lib_u8 vector, lib_u16 offset, lib_u16 selector, lib_u8 type,
    lib_u8 dpl, lib_u8 present)
{
    lib_u8 gate[8u] = {0};

    gate[0] = (lib_u8)offset;
    gate[1] = (lib_u8)(offset >> 8u);
    gate[2] = (lib_u8)selector;
    gate[3] = (lib_u8)(selector >> 8u);
    gate[5] = (present ? 0x80u : 0u) | (lib_u8)(dpl << 5u) | type;
    return test_protected_16_write(state, TEST_PROTECTED_16_IDT_BASE +
        (lib_u32)vector * 8u, gate, sizeof(gate));
}

/* Enter protected ring 0 through guest instructions only. */
static lib_i32 TEST_PROTECTED_16_UNUSED test_protected_16_prepare_with_planar_parity(
    test_protected_16_machine *state, core_machine_cpu_profile profile,
    lib_i32 planar_parity, lib_i32 default32)
{
    static const lib_u8 reset_jump[] = {0xeau,0u,0u,0u,0u};
    static const lib_u8 gdt_pointer[] = {0x37u,0u,0u,0x03u,0u,0u};
    static const lib_u8 idt_pointer[] = {0xffu,0x01u,0u,0x04u,0u,0u};
    static const lib_u8 source_gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0x20u,0u,0x9au,0u,0u,
        0xffu,0xffu,0u,0u,0u,0x92u,0u,0u,
        0xffu,0xffu,0u,0x20u,0u,0xfau,0u,0u,
        0xffu,0xffu,0u,0u,0u,0xf2u,0u,0u,
        0x67u,0u,0u,0x05u,0u,0x89u,0u,0u,
        0u,0u,0u,0u,0u,0u,0u,0u
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0x0fu,0x01u,0x1eu,0x20u,0x01u,
        0xb8u,0x01u,0u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0u,0x8eu,0xd8u,0x8eu,0xc0u,
        0x8eu,0xd0u,0xbcu,0u,0x80u,
        0xeau,0u,0u,0x08u,0u
    };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_planar_parity_config parity = {
        .port = CORE_MACHINE_PC_AT_PORT_B,
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .refresh_status_source = CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1,
        .refresh_status_toggle_ticks = 0u
    };
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};
    lib_u8 gdt[sizeof(source_gdt)];

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    lib_memory_copy(gdt, source_gdt, sizeof(gdt));
    if (default32) {
        gdt[14u] = 0x40u;
        gdt[22u] = 0x40u;
        gdt[30u] = 0x40u;
        gdt[38u] = 0x40u;
    }
    if (profile == CORE_MACHINE_CPU_PROFILE_80286) {
        gdt[40u] = 0x2bu;
        gdt[45u] = 0x81u;
    }
    if (core_machine_create(&config, &state->machine) != LIB_STATUS_OK ||
        (planar_parity && core_machine_configure_planar_parity(state->machine,
            &parity) != LIB_STATUS_OK) ||
        core_machine_freeze_execution_providers(state->machine) != LIB_STATUS_OK ||
        core_machine_reset(state->machine) != LIB_STATUS_OK ||
        !test_protected_16_write(state, TEST_PROTECTED_16_RESET_PHYSICAL,
            reset_jump, sizeof(reset_jump)) ||
        !test_protected_16_write(state, TEST_PROTECTED_16_GDT_POINTER,
            gdt_pointer, sizeof(gdt_pointer)) ||
        !test_protected_16_write(state, TEST_PROTECTED_16_IDT_POINTER,
            idt_pointer, sizeof(idt_pointer)) ||
        !test_protected_16_write(state, TEST_PROTECTED_16_GDT_BASE, gdt,
            sizeof(gdt)) || !test_protected_16_write(state, 0u, bootstrap,
            sizeof(bootstrap)) || core_machine_run(state->machine,
            (core_machine_run_budget){11u,0u}, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET ||
        !test_protected_16_snapshot(state, &snapshot) ||
        (snapshot.cr0 & VCPU_CR0_PE) == 0u || snapshot.cs.selector != 0x0008u ||
        snapshot.cs.base != TEST_PROTECTED_16_CODE_BASE ||
        snapshot.ss.selector != 0x0010u || snapshot.esp != TEST_PROTECTED_16_STACK_TOP ||
        snapshot.eip != 0u) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

#define test_protected_16_prepare(state, profile) \
    test_protected_16_prepare_with_planar_parity((state), (profile), LIB_FALSE, LIB_FALSE)

#define test_protected_32_prepare(state) \
    test_protected_16_prepare_with_planar_parity((state), \
        CORE_MACHINE_CPU_PROFILE_80386, LIB_FALSE, LIB_TRUE)

/* The old fixtures wrote CPL 3 and TR caches directly.  This setup reaches
 * the same architectural state through LTR and IRET before exposing user code. */
static lib_i32 TEST_PROTECTED_16_UNUSED test_protected_16_enter_user_with_tss(
    test_protected_16_machine *state, core_machine_cpu_profile profile,
    lib_i32 tss32)
{
    static const lib_u8 tss16[] = {
        0u,0u,0u,0x80u,0x10u,0u
    };
    static const lib_u8 tss32_image[] = {
        0u,0u,0u,0u,0u,0x80u,0u,0u,0x10u,0u
    };
    static const lib_u8 kernel_to_user[] = {
        0xb8u,0x28u,0u,0x0fu,0x00u,0xd8u,
        0xb8u,0x23u,0u,0x50u,
        0xb8u,0u,0x70u,0x50u,
        0xb8u,0x02u,0x02u,0x50u,
        0xb8u,0x1bu,0u,0x50u,
        0xb8u,0u,0x01u,0x50u,0xcfu
    };
    static const lib_u8 user_loop[] = {0xebu,0xfeu};
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};
    lib_u8 tss_descriptor[] = {
        0x2bu,0u,0u,0x05u,0u,0x81u,0u,0u
    };

    if (profile == CORE_MACHINE_CPU_PROFILE_80286) tss32 = LIB_FALSE;
    if (tss32) {
        tss_descriptor[0] = 0x67u;
        tss_descriptor[5] = 0x89u;
    }
    return state != LIB_NULL && state->machine != LIB_NULL &&
        test_protected_16_write(state, TEST_PROTECTED_16_GDT_BASE + 40u,
            tss_descriptor, sizeof(tss_descriptor)) &&
        test_protected_16_write(state, 0x0500u,
            tss32 ? tss32_image : tss16,
            tss32 ? sizeof(tss32_image) : sizeof(tss16)) &&
        test_protected_16_write(state, TEST_PROTECTED_16_CODE_BASE,
            kernel_to_user, sizeof(kernel_to_user)) &&
        test_protected_16_write(state, TEST_PROTECTED_16_CODE_BASE + 0x0100u,
            user_loop, sizeof(user_loop)) && core_machine_run(state->machine,
            (core_machine_run_budget){13u,0u}, &result) == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        test_protected_16_snapshot(state, &snapshot) &&
        snapshot.cs.selector == 0x001bu && snapshot.ss.selector == 0x0023u &&
        snapshot.cs.dpl == 3u && snapshot.ss.dpl == 3u &&
        snapshot.tr.selector == 0x0028u && snapshot.tr.dpl == 0u &&
        snapshot.eip == 0x0100u && snapshot.esp == 0x7000u;
}

/* The 80386 default-32 setup mirrors the 16-bit path above, but keeps every
 * post-jump instruction correctly encoded for the descriptor's D bit. */
static lib_i32 TEST_PROTECTED_16_UNUSED test_protected_32_enter_user(
    test_protected_16_machine *state)
{
    static const lib_u8 tss32_image[] = {
        0u,0u,0u,0u,0u,0x80u,0u,0u,0x10u,0u
    };
    static const lib_u8 kernel_to_user[] = {
        0x66u,0xb8u,0x28u,0u,0x0fu,0x00u,0xd8u,
        0x68u,0x23u,0u,0u,0u,0x68u,0u,0x70u,0u,0u,
        0x68u,0x02u,0x02u,0u,0u,0x68u,0x1bu,0u,0u,0u,
        0x68u,0u,0x01u,0u,0u,0xcfu
    };
    static const lib_u8 user_loop[] = {0xebu,0xfeu};
    static const lib_u8 tss_descriptor[] = {
        0x67u,0u,0u,0x05u,0u,0x89u,0u,0u
    };
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};

    return state != LIB_NULL && state->machine != LIB_NULL &&
        test_protected_16_write(state, TEST_PROTECTED_16_GDT_BASE + 40u,
            tss_descriptor, sizeof(tss_descriptor)) &&
        test_protected_16_write(state, 0x0500u, tss32_image,
            sizeof(tss32_image)) && test_protected_16_write(state,
            TEST_PROTECTED_16_CODE_BASE, kernel_to_user,
            sizeof(kernel_to_user)) && test_protected_16_write(state,
            TEST_PROTECTED_16_CODE_BASE + 0x0100u, user_loop,
            sizeof(user_loop)) && core_machine_run(state->machine,
            (core_machine_run_budget){8u,0u}, &result) == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        test_protected_16_snapshot(state, &snapshot) &&
        snapshot.cs.selector == 0x001bu && snapshot.ss.selector == 0x0023u &&
        snapshot.cs.dpl == 3u && snapshot.ss.dpl == 3u &&
        snapshot.tr.selector == 0x0028u && snapshot.tr.dpl == 0u &&
        snapshot.eip == 0x0100u && snapshot.esp == 0x7000u;
}

#define test_protected_16_enter_user(state, profile) \
    test_protected_16_enter_user_with_tss((state), (profile), \
        (profile) == CORE_MACHINE_CPU_PROFILE_80386)

static void TEST_PROTECTED_16_UNUSED test_protected_16_destroy(test_protected_16_machine *state)
{
    if (state == LIB_NULL) return;
    core_machine_destroy(state->machine);
    state->machine = LIB_NULL;
}

#endif
