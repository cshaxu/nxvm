#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"

#define ATOMIC_GDT_POINTER 0x0100u
#define ATOMIC_IDT_POINTER 0x0110u
#define ATOMIC_GDT_BASE 0x0300u
#define ATOMIC_IDT_BASE 0x0400u
#define ATOMIC_KERNEL_BASE 0x2000u
#define ATOMIC_KERNEL_STACK_BASE 0x3000u
#define ATOMIC_USER_CODE_ACCESS (ATOMIC_GDT_BASE + 29u)
#define ATOMIC_NONPRESENT_CODE_ACCESS (ATOMIC_GDT_BASE + 45u)
#define ATOMIC_NONPRESENT_STACK_ACCESS (ATOMIC_GDT_BASE + 53u)
#define ATOMIC_BAD_ENTRY_ACCESS (ATOMIC_GDT_BASE + 61u)

typedef struct atomic_machine {
    core_machine *machine;
    t_cpu *cpu;
    core_machine_cpu_execution_context *execution;
} atomic_machine;

typedef struct atomic_return_case {
    const C_CHAR *name;
    uint16_t cs;
    uint16_t ss;
    uint8_t vector;
    uint16_t error_code;
    C_INT delivered;
} atomic_return_case;

static C_VOID atomic_reset(C_VOID *opaque)
{
    atomic_machine *state = (atomic_machine *)opaque;

    if (state == STD_NULL || state->cpu == STD_NULL || state->execution == STD_NULL) return;
    (C_VOID)core_machine_cpu_execution_load_segment(state->execution,
        &state->cpu->data.cs, 0u);
    (C_VOID)core_machine_cpu_execution_load_segment(state->execution,
        &state->cpu->data.ds, 0u);
    (C_VOID)core_machine_cpu_execution_load_segment(state->execution,
        &state->cpu->data.es, 0u);
    (C_VOID)core_machine_cpu_execution_load_segment(state->execution,
        &state->cpu->data.ss, 0u);
    state->cpu->data.eip = 0u;
}

static const core_machine_execution_provider atomic_provider = {
    atomic_reset, STD_NULL, STD_NULL
};

static C_INT atomic_prepare(atomic_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK) return 0;
    state->cpu = core_machine_configuration_cpu_borrow(state->machine);
    state->execution = core_machine_configuration_cpu_execution_borrow(state->machine);
    if (state->cpu == STD_NULL || state->execution == STD_NULL ||
        core_machine_bind_execution_provider(state->machine, &atomic_provider, state) !=
            TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != TYPE_STATUS_OK ||
        core_machine_reset(state->machine) != TYPE_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT atomic_write(atomic_machine *state, uint32_t address,
    const uint8_t *bytes, STD_SIZE_T byte_count)
{
    return core_machine_memory_write(state->machine, address, bytes, byte_count) ==
        TYPE_STATUS_OK;
}

static C_VOID atomic_set_gate(uint8_t *idt, uint8_t vector, uint16_t offset)
{
    uint16_t index = (uint16_t)vector * 8u;

    idt[index] = (uint8_t)offset;
    idt[index + 1u] = (uint8_t)(offset >> 8u);
    idt[index + 2u] = 0x08u;
    idt[index + 5u] = 0x86u;
}

static C_INT atomic_install(atomic_machine *state)
{
    static const uint8_t gdt_pointer[] = { 0x3fu, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u };
    static const uint8_t idt_pointer[] = { 0x6fu, 0x00u, 0x00u, 0x04u, 0x00u, 0x00u };
    static const uint8_t gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0xff,0xff,0,0x40,0,0xfa,0,0,
        0xff,0xff,0,0x50,0,0xf2,0,0,
        0xff,0xff,0,0x40,0,0x7a,0,0,
        0xff,0xff,0,0x50,0,0x72,0,0,
        0xff,0xff,0,0x60,0,0xf2,0,0
    };
    uint8_t idt[0x70u] = {0};
    static const uint8_t real_code[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0x0f,0x01,0x1e,0x10,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x10,0x00,0x8e,0xd8,0x8e,0xd0,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    static const uint8_t initial_halt[] = { 0xf4 };
    uint8_t fault_handlers[0x21u] = {0};

    atomic_set_gate(idt, 11u, 0x0110u);
    atomic_set_gate(idt, 12u, 0x0120u);
    atomic_set_gate(idt, 13u, 0x0100u);
    fault_handlers[0u] = 0xf4u;
    fault_handlers[0x10u] = 0xf4u;
    fault_handlers[0x20u] = 0xf4u;
    return atomic_write(state, ATOMIC_GDT_POINTER, gdt_pointer, sizeof(gdt_pointer)) &&
        atomic_write(state, ATOMIC_IDT_POINTER, idt_pointer, sizeof(idt_pointer)) &&
        atomic_write(state, ATOMIC_GDT_BASE, gdt, sizeof(gdt)) &&
        atomic_write(state, ATOMIC_IDT_BASE, idt, sizeof(idt)) &&
        atomic_write(state, 0u, real_code, sizeof(real_code)) &&
        atomic_write(state, ATOMIC_KERNEL_BASE, initial_halt, sizeof(initial_halt)) &&
        atomic_write(state, ATOMIC_KERNEL_BASE + 0x100u, fault_handlers,
            sizeof(fault_handlers));
}

static STD_SIZE_T atomic_return_code(uint8_t *bytes, C_INT use_iret,
    const atomic_return_case *test)
{
    static const uint8_t retf[] = {
        0xb8,0,0,0x50,0xb8,0x00,0xa0,0x50,
        0xb8,0,0,0x50,0xb8,0x00,0x00,0x50,0xcb
    };
    static const uint8_t iret[] = {
        0xb8,0,0,0x50,0xb8,0x00,0xa0,0x50,
        0xb8,0x02,0x02,0x50,0xb8,0,0,0x50,
        0xb8,0x00,0x00,0x50,0xcf
    };

    if (use_iret) {
        STD_MEMCPY(bytes, iret, sizeof(iret));
        bytes[1u] = (uint8_t)test->ss;
        bytes[2u] = (uint8_t)(test->ss >> 8u);
        bytes[13u] = (uint8_t)test->cs;
        bytes[14u] = (uint8_t)(test->cs >> 8u);
        return sizeof(iret);
    }
    STD_MEMCPY(bytes, retf, sizeof(retf));
    bytes[1u] = (uint8_t)test->ss;
    bytes[2u] = (uint8_t)(test->ss >> 8u);
    bytes[9u] = (uint8_t)test->cs;
    bytes[10u] = (uint8_t)(test->cs >> 8u);
    return sizeof(retf);
}

static uint16_t atomic_fault_stop_ip(uint8_t vector)
{
    switch (vector) {
    case 11u: return 0x0111u;
    case 12u: return 0x0121u;
    case 13u: return 0x0101u;
    default: return 0u;
    }
}

static C_INT atomic_test_outer_return(const atomic_return_case *test, C_INT use_iret)
{
    static const uint32_t access_addresses[] = {
        ATOMIC_USER_CODE_ACCESS,
        ATOMIC_NONPRESENT_CODE_ACCESS,
        ATOMIC_NONPRESENT_STACK_ACCESS,
        ATOMIC_BAD_ENTRY_ACCESS
    };
    static const uint8_t expected_access[] = { 0xfau, 0x7au, 0x72u, 0xf2u };
    atomic_machine state;
    core_machine_run_result result = {0};
    const core_machine_run_budget budget = { 128u, 0u };
    t_cpu_data before;
    uint8_t code[21u];
    uint8_t access[sizeof(access_addresses) / sizeof(access_addresses[0])];
    uint16_t frame[4u] = {0};
    uint16_t expected_sp = use_iret ? 0x7feeu : 0x7ff0u;
    uint16_t expected_return_ip = use_iret ? 20u : 16u;
    STD_SIZE_T index;
    C_INT install_failed;
    C_INT boot_failed;
    C_INT run_failed;
    C_INT access_failed = 0;
    C_INT state_failed;
    C_INT failed = !atomic_prepare(&state);

    if (!failed) {
        install_failed = !atomic_install(&state);
        boot_failed = core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= install_failed || boot_failed;
        state.cpu->data.flagHalt = TYPE_FALSE;
        state.cpu->data.eip = 0u;
        before = state.cpu->data;
        failed |= !atomic_write(&state, ATOMIC_KERNEL_BASE, code,
            atomic_return_code(code, use_iret, test));
        if (test->delivered) {
            run_failed = core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        } else {
            run_failed = core_machine_run(state.machine, budget, &result) != TYPE_STATUS_FAULT ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                result.detail != 0x00001000u;
        }
        failed |= run_failed;
        for (index = 0u; index < sizeof(access); ++index) {
            access_failed |= core_machine_cpu_execution_read_linear(state.execution,
                access_addresses[index], TYPE_REFERENCE_OF(access[index]),
                sizeof(access[index])) ||
                access[index] != expected_access[index];
        }
        failed |= access_failed;
        if (test->delivered) {
            state_failed = core_machine_memory_read(state.machine,
                ATOMIC_KERNEL_STACK_BASE + expected_sp, frame, sizeof(frame)) != TYPE_STATUS_OK;
            state_failed |= state.cpu->data.eip != atomic_fault_stop_ip(test->vector) ||
                state.cpu->data.sp != expected_sp ||
                STD_MEMCMP(&state.cpu->data.cs, &before.cs, sizeof(before.cs)) != 0 ||
                STD_MEMCMP(&state.cpu->data.ss, &before.ss, sizeof(before.ss)) != 0 ||
                frame[0u] != test->error_code || frame[1u] != expected_return_ip ||
                frame[2u] != before.cs.selector || frame[3u] != before.flags;
        } else {
            state_failed = state.cpu->data.eip != expected_return_ip ||
                state.cpu->data.sp != (use_iret ? 0x7ff6u : 0x7ff8u) ||
                state.cpu->data.flags != before.flags ||
                STD_MEMCMP(&state.cpu->data.cs, &before.cs, sizeof(before.cs)) != 0 ||
                STD_MEMCMP(&state.cpu->data.ss, &before.ss, sizeof(before.ss)) != 0;
        }
        failed |= state_failed;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T293 %s return=%s fail=%d/%d/%d/%d/%d ip=%04x sp=%04x flags=%04x/%04x cs=%d ss=%d access=%02x/%02x/%02x/%02x frame=%04x/%04x/%04x/%04x\n",
                test->name, use_iret ? "iret" : "retf", install_failed, boot_failed,
                run_failed, access_failed, state_failed,
                state.cpu->data.ip, state.cpu->data.sp,
                state.cpu->data.flags, before.flags,
                STD_MEMCMP(&state.cpu->data.cs, &before.cs, sizeof(before.cs)),
                STD_MEMCMP(&state.cpu->data.ss, &before.ss, sizeof(before.ss)),
                access[0u], access[1u], access[2u], access[3u],
                frame[0u], frame[1u], frame[2u], frame[3u]);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

C_INT main(C_VOID)
{
    static const atomic_return_case cases[] = {
        { "nonpresent-cs", 0x002bu, 0x0023u, 11u, 0x0028u, 1 },
        { "nonpresent-ss", 0x001bu, 0x0033u, 12u, 0x0030u, 0 },
        { "inaccessible-entry", 0x003bu, 0x0023u, 13u, 0x0038u, 1 }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        if (atomic_test_outer_return(&cases[index], 0) ||
            atomic_test_outer_return(&cases[index], 1)) return 1;
    }
    STD_PRINTF("M5:T293:S2:PROTECTED-RETURN-ATOMICITY:OK\n");
    return 0;
}
