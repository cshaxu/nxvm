#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"
#include "core/machine/memory_interface.h"
#include "core/machine/port_interface.h"

#define GDT_PTR 0x0100u
#define IDT_PTR 0x0110u
#define GDT_BASE 0x0300u
#define IDT_BASE 0x0400u
#define TSS_BASE 0x0600u
#define KERNEL_BASE 0x2000u
#define USER_CODE_BASE 0x4000u
#define USER_DATA_BASE 0x5000u

typedef enum iomap_case {
    IOMAP_CASE_ALLOW = 0,
    IOMAP_CASE_DENY_IN,
    IOMAP_CASE_DENY_OUT,
    IOMAP_CASE_TRUNCATED_WORD,
    IOMAP_CASE_IOPL_BYPASS
} iomap_case;

typedef struct iomap_port_state {
    type_unsigned_32 reads;
    type_unsigned_32 writes;
    type_unsigned_32 last_write;
} iomap_port_state;

typedef struct iomap_machine {
    core_machine *machine;
    iomap_port_state port;
} iomap_machine;

static C_VOID iomap_reset(C_VOID *opaque)
{
    iomap_machine *state = (iomap_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static type_status iomap_port_read(C_VOID *opaque, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    iomap_port_state *state = (iomap_port_state *)opaque;

    if (state == STD_NULL || out_value == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    ++state->reads;
    *out_value = port == 0x00e0u ? 0x7cu : 0x5du;
    return TYPE_STATUS_OK;
}

static type_status iomap_port_write(C_VOID *opaque, type_unsigned_16 port,
    type_unsigned_32 value)
{
    iomap_port_state *state = (iomap_port_state *)opaque;

    if (state == STD_NULL || (port != 0x00e0u && port != 0x00e1u)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ++state->writes;
    state->last_write = value;
    return TYPE_STATUS_OK;
}

static const core_machine_execution_provider iomap_execution_provider = {
    iomap_reset, STD_NULL
};

static const core_machine_port_provider iomap_port_provider = {
    iomap_port_read, iomap_port_write
};

static C_INT write_bytes(core_machine *machine, type_unsigned_32 address,
    const type_unsigned_8 *bytes, STD_SIZE_T count)
{
    return core_machine_memory_write(machine, address, bytes, count) ==
        TYPE_STATUS_OK;
}

static C_INT iomap_prepare(iomap_machine *state, core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK) return 0;
    if (core_machine_install_port_provider(state->machine, 0x00e0u, 0x00e1u,
            &iomap_port_provider, &state->port) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &iomap_execution_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT iomap_install(iomap_machine *state, core_machine_cpu_profile profile,
    iomap_case test_case)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x2fu,0,0,3,0,0 };
    static const type_unsigned_8 idt_pointer[] = { 0x97u,0x01u,0,4,0,0 };
    type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0xff,0xff,0,0x40,0,0xfa,0,0,
        0xff,0xff,0,0x50,0,0xf2,0,0,
        0xff,0,0,0x06,0,0x89,0,0
    };
    static const type_unsigned_8 real_code[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0x0f,0x01,0x1e,0x10,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x28,0x00,0x0f,0x00,0xd8,
        0xb8,0x10,0x00,0x8e,0xd0,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    type_unsigned_8 kernel_entry[] = {
        0xb8,0x10,0x00,0x8e,0xd8,
        0xb8,0x23,0x00,0x50,
        0xb8,0x00,0xa0,0x50,
        0xb8,0x02,0x02,0x50,
        0xb8,0x1b,0x00,0x50,
        0xb8,0x00,0x00,0x50,
        0xb8,0x23,0x00,0x8e,0xd8,
        0xcf
    };
    static const type_unsigned_8 kernel_stop[] = { 0xf4 };
    static const type_unsigned_8 kernel_fault[] = {
        0xb8,0x33,0x33,0xa3,0x04,0x00,0xf4
    };
    static const type_unsigned_8 user_allow[] = {
        0xe4,0xe0,0xa2,0x00,0x00,0xb0,0x5a,0xe6,0xe0,0xcd,0x31
    };
    static const type_unsigned_8 user_deny_in[] = { 0xe4,0xe0 };
    static const type_unsigned_8 user_deny_out[] = { 0xb0,0x5a,0xe6,0xe0 };
    static const type_unsigned_8 user_truncated_word[] = { 0xe5,0xe1 };
    type_unsigned_8 idt[0x198u] = {0};
    type_unsigned_8 iomap_byte = 0u;
    type_unsigned_16 iomap_base = 0x0080u;
    type_unsigned_16 ss0 = 0x0010u;
    type_unsigned_32 esp0 = 0x00009000u;
    type_unsigned_32 tss_limit = 0x00ffu;
    const type_unsigned_8 *user_code = user_allow;
    STD_SIZE_T user_code_bytes = sizeof(user_allow);

    idt[0x68u] = 0x20u;
    idt[0x69u] = 0x01u;
    idt[0x6au] = 0x08u;
    idt[0x6du] = 0x86u;
    idt[0x188u] = 0x10u;
    idt[0x189u] = 0x01u;
    idt[0x18au] = 0x08u;
    idt[0x18du] = 0xe6u;

    switch (test_case) {
    case IOMAP_CASE_DENY_IN:
        iomap_byte = 0x01u;
        user_code = user_deny_in;
        user_code_bytes = sizeof(user_deny_in);
        break;
    case IOMAP_CASE_DENY_OUT:
        iomap_byte = 0x01u;
        user_code = user_deny_out;
        user_code_bytes = sizeof(user_deny_out);
        break;
    case IOMAP_CASE_TRUNCATED_WORD:
        iomap_base = 0x0080u;
        tss_limit = 0x0080u;
        user_code = user_truncated_word;
        user_code_bytes = sizeof(user_truncated_word);
        break;
    case IOMAP_CASE_IOPL_BYPASS:
        iomap_byte = 0x01u;
        kernel_entry[14] = 0x02u;
        kernel_entry[15] = 0x32u;
        break;
    default:
        break;
    }
    if (profile == CORE_MACHINE_CPU_PROFILE_80286) {
        type_unsigned_16 sp0 = 0x9000u;

        gdt[45] = 0x81u;
        if (!write_bytes(state->machine, TSS_BASE + 2u,
                (const type_unsigned_8 *)&sp0, sizeof(sp0)) ||
            !write_bytes(state->machine, TSS_BASE + 4u,
                (const type_unsigned_8 *)&ss0, sizeof(ss0))) return 0;
    } else if (!write_bytes(state->machine, TSS_BASE + 4u,
            (const type_unsigned_8 *)&esp0, sizeof(esp0)) ||
        !write_bytes(state->machine, TSS_BASE + 8u,
            (const type_unsigned_8 *)&ss0, sizeof(ss0))) {
        return 0;
    }
    gdt[40] = (type_unsigned_8)tss_limit;
    gdt[41] = (type_unsigned_8)(tss_limit >> 8u);
    return write_bytes(state->machine, GDT_PTR, gdt_pointer, sizeof(gdt_pointer)) &&
        write_bytes(state->machine, IDT_PTR, idt_pointer, sizeof(idt_pointer)) &&
        write_bytes(state->machine, GDT_BASE, gdt, sizeof(gdt)) &&
        write_bytes(state->machine, IDT_BASE, idt, sizeof(idt)) &&
        write_bytes(state->machine, TSS_BASE + 0x66u,
            (const type_unsigned_8 *)&iomap_base, sizeof(iomap_base)) &&
        write_bytes(state->machine, TSS_BASE + iomap_base + 0x1cu,
            &iomap_byte, sizeof(iomap_byte)) &&
        write_bytes(state->machine, 0u, real_code, sizeof(real_code)) &&
        write_bytes(state->machine, KERNEL_BASE, kernel_entry, sizeof(kernel_entry)) &&
        write_bytes(state->machine, KERNEL_BASE + 0x110u, kernel_stop,
            sizeof(kernel_stop)) &&
        write_bytes(state->machine, KERNEL_BASE + 0x120u, kernel_fault,
            sizeof(kernel_fault)) &&
        write_bytes(state->machine, USER_CODE_BASE, user_code, user_code_bytes);
}

static C_INT iomap_run_case(core_machine_cpu_profile profile, iomap_case test_case)
{
    iomap_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_16 marker[3] = {0u, 0u, 0u};
    const core_machine_run_budget budget = { 1024u, 0u };
    C_INT failed = !iomap_prepare(&state, profile);
    C_INT denied = test_case == IOMAP_CASE_DENY_IN ||
        test_case == IOMAP_CASE_DENY_OUT ||
        test_case == IOMAP_CASE_TRUNCATED_WORD;

    if (!failed) {
        failed |= !iomap_install(&state, profile, test_case);
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= core_machine_memory_read(state.machine, USER_DATA_BASE, marker,
            sizeof(marker)) != TYPE_STATUS_OK;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        if (denied) {
            failed |= state.port.reads != 0u || state.port.writes != 0u ||
                marker[2] != 0x3333u || diagnostic.first_fault.valid ||
                !diagnostic.last_delivered_exception.valid ||
                !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                    VCPUINS_EXCEPT_GP) ||
                diagnostic.last_delivered_exception.exception_code != 0u;
        } else {
            failed |= state.port.reads != 1u || state.port.writes != 1u ||
                state.port.last_write != 0x5au || marker[0] != 0x007cu ||
                diagnostic.first_fault.valid ||
                diagnostic.last_delivered_exception.valid;
        }
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T260 case=%u result=%u reads=%u writes=%u marker=%04x/%04x/%04x delivered=%d/%u code=%04x\n",
                (unsigned)test_case, (unsigned)result.reason,
                (unsigned)state.port.reads, (unsigned)state.port.writes,
                marker[0], marker[1], marker[2],
                diagnostic.last_delivered_exception.valid,
                diagnostic.last_delivered_exception.exception_mask,
                diagnostic.last_delivered_exception.exception_code);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

int main(void)
{
    C_INT failed = 0;

    failed |= iomap_run_case(CORE_MACHINE_CPU_PROFILE_80386, IOMAP_CASE_ALLOW);
    failed |= iomap_run_case(CORE_MACHINE_CPU_PROFILE_80386, IOMAP_CASE_DENY_IN);
    failed |= iomap_run_case(CORE_MACHINE_CPU_PROFILE_80386, IOMAP_CASE_DENY_OUT);
    failed |= iomap_run_case(CORE_MACHINE_CPU_PROFILE_80386,
        IOMAP_CASE_TRUNCATED_WORD);
    failed |= iomap_run_case(CORE_MACHINE_CPU_PROFILE_80386,
        IOMAP_CASE_IOPL_BYPASS);
    failed |= iomap_run_case(CORE_MACHINE_CPU_PROFILE_80286,
        IOMAP_CASE_IOPL_BYPASS);
    failed |= iomap_run_case(CORE_MACHINE_CPU_PROFILE_80286,
        IOMAP_CASE_DENY_IN);
    if (failed) return 1;
    STD_PRINTF("M5:T260:S3:TSS-IOMAP:CORPUS:OK\n");
    return 0;
}
