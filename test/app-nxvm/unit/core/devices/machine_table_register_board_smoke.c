#include "support/core_machine_board_fixture.h"
#include "support/pic_fixture.h"
#include "x86/devices/cpu/cpu.h"
#include "app-nxvm/devices/device_support.h"
#include "app-nxvm/devices/machine_interface.h"
#include <stdio.h>

static lib_i32 table_register_board_create(core_machine **out_machine)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    core_machine *machine = LIB_NULL;

    *out_machine = LIB_NULL;
    if (core_machine_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 table_register_board_case(const lib_u8 *opcode,
    lib_u8 opcode_bytes, lib_bool lidt)
{
    static const lib_u16 irq_vector[] = {0x0100u, 0u};
    static const lib_u8 handler = 0xf4u;
    const core_machine_debug_register_patch patch = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {
            [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
            [CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_IF
        }
    };
    lib_u8 program[8] = {0};
    lib_u8 table[6] = {0x57u,0x13u,0x56u,0x34u,0x12u,0u};
    core_machine *machine = LIB_NULL;
    core_machine_pic_irq_source source = {0};
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_i32 failed = !table_register_board_create(&machine);

    lib_memory_copy(program, opcode, opcode_bytes);
    program[opcode_bytes] = 0x90u;
    if (lidt) {
        table[2] = 0u;
        table[3] = 0u;
        table[4] = 0u;
    }
    if (!failed)
        failed = core_machine_debug_patch_registers(machine, &patch) !=
                LIB_STATUS_OK || core_machine_memory_write(machine, 0u,
                program, opcode_bytes + 1u) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0200u, table, sizeof(table)) !=
                LIB_STATUS_OK || core_machine_memory_write(machine, 0x20u * 4u,
                irq_vector, sizeof(irq_vector)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0100u, &handler,
                sizeof(handler)) != LIB_STATUS_OK;
    if (!failed) {
        test_pic_program_vector(&machine->shared_pic_master, 0x20u);
        core_machine_pic_irq_source_bind(&source, &machine->shared_pic_master,
            &machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed = core_machine_run(machine, (core_machine_run_budget){2u,0u},
            &result) != LIB_STATUS_OK || core_machine_get_cpu_diagnostic(machine,
            &diagnostic) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
    }
    if (!failed)
        failed = diagnostic.first_fault.valid || result.reason !=
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT || after.eip != 0x0101u ||
            !(test_pic_read(&machine->shared_pic_master, 0x0bu) &
                VPIC_ISR_IRQ(0u)) ||
            (test_pic_read(&machine->shared_pic_master, 0x0au) & VPIC_IRR_IRQ(0u));
    if (!failed && !lidt)
        failed = result.executed != 2u;
    core_machine_destroy(machine);
    return !failed;
}

/* LTR m16 consumes a board-provided descriptor.  Keep that wiring here,
 * rather than manufacturing a GDT inside the CPU-only instruction fixture. */
static lib_i32 table_register_board_ltr_memory(void)
{
    static const lib_u8 gdt_pointer[] = {0x27u,0u,0u,0x03u,0u,0u};
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0,
        0x1fu,0,0,0x50u,0,0x82u,0,0,
        0x67u,0,0,0x60u,0,0x89u,0,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0x8eu,0xd0u,
        0xbcu,0,0x80u,0xeau,0,0,0x08u,0
    };
    static const lib_u8 ltr[] = {0x0fu,0,0x1eu,0,0x40u,0xf4u};
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
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP),
        .values = {[CORE_MACHINE_DEBUG_ESP] = 0x8000u}
    };
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_u16 selector = 0x20u;
    lib_u8 access = 0u;
    lib_i32 failed = core_machine_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK;

    if (!failed)
        failed = core_machine_memory_write(machine, 0x0100u, gdt_pointer,
            sizeof(gdt_pointer)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0300u, gdt, sizeof(gdt)) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0u, bootstrap, sizeof(bootstrap)) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x2000u, ltr, sizeof(ltr)) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x4000u, &selector,
                sizeof(selector)) != LIB_STATUS_OK ||
            core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){64u,0u},
                &result) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine, 0x0325u, &access,
                sizeof(access)) != LIB_STATUS_OK;
    if (!failed)
        failed = result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            after.tr.selector != 0x20u ||
            after.tr.type != VCPU_DESC_SYS_TYPE_TSS_32_BUSY || access != 0x8bu;
    core_machine_destroy(machine);
    return !failed;
}

/* Segment selection is board-visible execution context.  Set SS/ES through
 * guest instructions, then prove LGDT/LIDT consume the routed memory image. */
static lib_i32 table_register_board_segment_sources(void)
{
    static const lib_u8 ss_code[] = {
        0xb8u,0x40u,0,0x8eu,0xd0u,0xbdu,0x20u,0,
        0x0fu,0x01u,0x56u,0x10u,0xf4u
    };
    static const lib_u8 es_code[] = {
        0xb8u,0x80u,0,0x8eu,0xc0u,0x26u,
        0x0fu,0x01u,0x16u,0,0x03u,0xf4u
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP),
        .values = {[CORE_MACHINE_DEBUG_ESP] = 0x8000u}
    };
    const lib_u8 *codes[] = {ss_code, es_code};
    const lib_size code_bytes[] = {sizeof(ss_code), sizeof(es_code)};
    const lib_u32 sources[] = {0x0430u, 0x0b00u};
    lib_u8 form, operation;

    for (operation = 0u; operation != 2u; ++operation)
        for (form = 0u; form != 2u; ++form) {
            lib_u8 image[6] = {0x57u,0x13u,0xefu,0xcdu,0xabu,0u};
            lib_u8 program[sizeof(ss_code)] = {0};
            core_machine *machine = LIB_NULL;
            core_machine_run_result result = {0};
            core_machine_debug_cpu_snapshot after = {0};
            lib_i32 failed = !table_register_board_create(&machine);

            lib_memory_copy(program, codes[form], code_bytes[form]);
            program[form == 0u ? 10u : 8u] |= operation << 3u;
            if (!failed)
                failed = core_machine_debug_patch_registers(machine, &entry) !=
                    LIB_STATUS_OK || core_machine_memory_write(machine, 0u,
                    program, code_bytes[form]) != LIB_STATUS_OK ||
                    core_machine_memory_write(machine, sources[form], image,
                        sizeof(image)) != LIB_STATUS_OK ||
                    core_machine_run(machine, (core_machine_run_budget){16u,0u},
                        &result) != LIB_STATUS_OK ||
                    core_machine_debug_capture_cpu_snapshot(machine,
                        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            if (!failed)
                failed = result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                    (operation == 0u && (after.gdtr.limit != 0x1357u ||
                    after.gdtr.base != 0x00abcdefu)) ||
                    (operation == 1u && (after.idtr.limit != 0x1357u ||
                    after.idtr.base != 0x00abcdefu));
            core_machine_destroy(machine);
            if (failed) return 0;
        }
    return 1;
}

/* A table image which crosses the DS limit must fault before either table
 * register is committed.  The #GP gate is part of the board contract. */
static lib_i32 table_register_board_source_limit(void)
{
    static const lib_u8 gdt_pointer[] = {0x1fu,0,0,0x03u,0,0};
    static const lib_u8 idt_pointer[] = {0x07u,0x01u,0,0x04u,0,0};
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0x03u,0x02u,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x18u,0,0x8eu,0xd0u,0xbcu,0,0x80u,
        0xb8u,0x10u,0,0x8eu,0xd8u,0xeau,0,0,0x08u,0
    };
    static const lib_u8 load_target[] = {0x0fu,0x01u,0x16u,0,0x02u,0xf4u};
    static const lib_u8 store_target[] = {0x0fu,0x01u,0x06u,0x0eu,0,0xf4u};
    static const lib_u8 handler = 0xf4u;
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
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP),
        .values = {[CORE_MACHINE_DEBUG_ESP] = 0x8000u}
    };
    lib_u8 idt[0x108u] = {0};
    lib_u8 source[6] = {0x57u,0x13u,0xefu,0xcdu,0xabu,0u};
    lib_u8 kind, operation;

    idt[13u * 8u + 1u] = 0x01u;
    idt[13u * 8u + 2u] = 0x08u;
    idt[13u * 8u + 5u] = 0x8eu;
    for (kind = 0u; kind != 2u; ++kind) for (operation = 0u;
        operation != 2u; ++operation) {
        core_machine *machine = LIB_NULL;
        core_machine_run_result result = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        core_machine_debug_cpu_snapshot after = {0};
        lib_u8 code[sizeof(load_target)];
        lib_u8 gdt_image[sizeof(gdt)];
        lib_u8 guard[6] = {0x3cu,0x3cu,0x3cu,0x3cu,0x3cu,0x3cu};
        lib_i32 failed = core_machine_create(&config, &machine) != LIB_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK;

        lib_memory_copy(code, kind ? store_target : load_target, sizeof(code));
        code[2] |= operation << 3u;
        lib_memory_copy(gdt_image, gdt, sizeof(gdt_image));
        if (kind) {
            gdt_image[16u] = 0x11u;
            gdt_image[17u] = 0u;
        }
        if (!failed)
            failed = core_machine_debug_patch_registers(machine, &entry) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x0100u, gdt_pointer,
                    sizeof(gdt_pointer)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x0110u, idt_pointer,
                    sizeof(idt_pointer)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x0300u, gdt_image,
                    sizeof(gdt_image)) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x0400u, idt, sizeof(idt)) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0u, bootstrap,
                    sizeof(bootstrap)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x2000u, code,
                    sizeof(code)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x2100u, &handler,
                    sizeof(handler)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x4200u, source,
                    sizeof(source)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x400eu, guard,
                    sizeof(guard)) != LIB_STATUS_OK ||
                test_core_machine_fixture_run_after_delivery(machine,
                    (core_machine_run_budget){64u,0u}, &result) != LIB_STATUS_OK ||
                core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                    LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed && kind)
            failed = core_machine_debug_read_memory(machine, 0x400eu, guard,
                sizeof(guard)) != LIB_STATUS_OK;
        if (!failed)
            failed = result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                diagnostic.first_fault.valid ||
                !diagnostic.last_delivered_exception.valid ||
                !(diagnostic.last_delivered_exception.exception_mask &
                    VCPUINS_EXCEPT_GP) || after.eip != 0x0101u ||
                (!kind && operation == 0u && (after.gdtr.limit != 0x001fu ||
                    after.gdtr.base != 0x00000300u)) ||
                (!kind && operation == 1u && (after.idtr.limit != 0x0107u ||
                    after.idtr.base != 0x00000400u)) ||
                (kind && (guard[0] != 0x3cu || guard[1] != 0x3cu ||
                    guard[2] != 0x3cu || guard[3] != 0x3cu ||
                    guard[4] != 0x3cu || guard[5] != 0x3cu));
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

/* This is the complete DOS discriminator, not a helper-level assertion:
 * 80286 stores FF in SGDT's sixth byte while 80386 preserves the high byte. */
static lib_i32 table_register_board_dos_sgdt_discriminator(void)
{
    static const lib_u8 code[] = {
        0x9cu,0x58u,0x25u,0,0xf0u,0x3du,0,0xf0u,0x75u,0,
        0xc8u,0x06u,0,0,0x0fu,0x01u,0x46u,0xfau,
        0x80u,0x7eu,0xffu,0xffu,0xc9u,0xbau,0x86u,0x03u,
        0x75u,0x03u,0xbau,0x86u,0x02u,0xf4u
    };
    static const struct {
        core_machine_cpu_profile profile;
        lib_u16 expected_dx;
    } cases[] = {
        {CORE_MACHINE_CPU_PROFILE_80286, 0x0286u},
        {CORE_MACHINE_CPU_PROFILE_80386, 0x0386u}
    };
    lib_u8 index;

    for (index = 0u; index != sizeof(cases) / sizeof(cases[0]); ++index) {
        const core_machine_config config = {
            .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
            .cpu_profile = cases[index].profile,
            .fpu_profile = X86_FPU_PROFILE_NONE
        };
        const core_machine_debug_register_patch entry = {
            .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBP),
            .values = {
                [CORE_MACHINE_DEBUG_ESP] = 0x7000u,
                [CORE_MACHINE_DEBUG_EBP] = 0x0200u
            }
        };
        core_machine *machine = LIB_NULL;
        core_machine_run_result result = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        core_machine_debug_cpu_snapshot after = {0};
        lib_i32 failed = core_machine_create(&config, &machine) != LIB_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK;

        if (!failed)
            failed = core_machine_debug_patch_registers(machine, &entry) !=
                LIB_STATUS_OK || core_machine_memory_write(machine, 0u, code,
                sizeof(code)) != LIB_STATUS_OK || core_machine_run(machine,
                (core_machine_run_budget){32u,0u}, &result) != LIB_STATUS_OK ||
                core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                    LIB_STATUS_OK || core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed)
            failed = result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                diagnostic.first_fault.valid || (lib_u16)after.edx !=
                    cases[index].expected_dx;
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

/* The consumer selector is loaded by the guest after LGDT.  No test-side
 * register patch may stand in for the instruction sequence under test. */
static lib_i32 table_register_board_lgdt_consumer(void)
{
    static const lib_u8 gdt_pointer[] = {0x1fu,0,0,0x03u,0,0};
    static const lib_u8 idt_pointer[] = {0x07u,0x01u,0,0x04u,0,0};
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x18u,0,0x8eu,0xd0u,0xbcu,0,0x80u,
        0xb8u,0x10u,0,0x8eu,0xd8u,0xeau,0,0,0x08u,0
    };
    static const lib_u8 target[] = {
        0xb8u,0x08u,0,0x0fu,0x01u,0x16u,0,0x02u,0x8eu,0xd8u,0xf4u
    };
    static const lib_u8 new_gdt_pointer[] = {0x0fu,0,0,0x05u,0,0};
    static const lib_u8 new_data_descriptor[] = {
        0xffu,0x0fu,0,0x40u,0,0x92u,0,0
    };
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
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP),
        .values = {[CORE_MACHINE_DEBUG_ESP] = 0x8000u}
    };
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_u8 idt[0x108u] = {0};
    lib_i32 failed = core_machine_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK;

    idt[13u * 8u + 1u] = 0x01u;
    idt[13u * 8u + 2u] = 0x08u;
    idt[13u * 8u + 5u] = 0x8eu;
    if (!failed)
        failed = core_machine_debug_patch_registers(machine, &entry) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0100u, gdt_pointer,
                sizeof(gdt_pointer)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0110u, idt_pointer,
                sizeof(idt_pointer)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0300u, gdt, sizeof(gdt)) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0400u, idt, sizeof(idt)) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0u, bootstrap,
                sizeof(bootstrap)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x2000u, target,
                sizeof(target)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x4200u, new_gdt_pointer,
                sizeof(new_gdt_pointer)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0508u, new_data_descriptor,
                sizeof(new_data_descriptor)) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){64u,0u},
                &result) != LIB_STATUS_OK || core_machine_get_cpu_diagnostic(machine,
                &diagnostic) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
    if (!failed)
        failed = result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            diagnostic.first_fault.valid || after.gdtr.limit != 0x000fu ||
            after.gdtr.base != 0x00000500u || after.ds.selector != 0x0008u ||
            after.ds.base != 0x00004000u || after.ds.limit != 0x00000fffu ||
            !after.ds.writable;
    core_machine_destroy(machine);
    return !failed;
}

/* Establish user CPL through the real outer IRET path.  The user instruction
 * must fault through the ring-0 #GP gate; debug never manufactures CPL 3. */
static lib_i32 table_register_board_cpl_reject(void)
{
    static const lib_u8 gdt_pointer[] = {0x2fu,0,0,0x03u,0,0};
    static const lib_u8 idt_pointer[] = {0x07u,0x01u,0,0x04u,0,0};
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0,
        0xffu,0xffu,0,0x30u,0,0xfau,0,0,
        0xffu,0xffu,0,0,0,0xf2u,0,0,
        0x2bu,0,0,0x05u,0,0x81u,0,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0,0x8eu,0xd0u,0xbcu,0,0x90u,
        0xb8u,0x28u,0,0x0fu,0,0xd8u,
        0x68u,0x23u,0,0x68u,0,0x80u,0x9cu,
        0x68u,0x1bu,0,0x68u,0,0xcfu
    };
    static const lib_u8 handler = 0xf4u;
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile, operation;

    for (profile = 0u; profile != 2u; ++profile) for (operation = 0u;
        operation != 2u; ++operation) {
        const core_machine_config config = {
            .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
            .cpu_profile = profiles[profile], .fpu_profile = X86_FPU_PROFILE_NONE
        };
        const core_machine_debug_register_patch entry = {
            .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP),
            .values = {[CORE_MACHINE_DEBUG_ESP] = 0x9000u}
        };
        lib_u8 user_code[] = {
            0xb8u,0x23u,0,0x8eu,0xd8u,0x0fu,0x01u,0x16u,0,0x06u
        };
        lib_u8 idt[0x108u] = {0};
        lib_u8 tss[6] = {0};
        lib_u8 source[6] = {0x5au,0x5au,0x5au,0x5au,0x5au,0x5au};
        core_machine *machine = LIB_NULL;
        core_machine_run_result result = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        core_machine_debug_cpu_snapshot after = {0};
        lib_u16 sp0 = 0x9000u, ss0 = 0x0010u;
        lib_i32 failed = core_machine_create(&config, &machine) != LIB_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK;

        user_code[7u] |= operation << 3u;
        idt[13u * 8u + 1u] = 0x01u;
        idt[13u * 8u + 2u] = 0x08u;
        idt[13u * 8u + 5u] = profiles[profile] ==
            CORE_MACHINE_CPU_PROFILE_80286 ? 0x86u : 0x8eu;
        lib_memory_copy(tss + 2u, &sp0, sizeof(sp0));
        lib_memory_copy(tss + 4u, &ss0, sizeof(ss0));
        if (!failed)
            failed = core_machine_debug_patch_registers(machine, &entry) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x0100u, gdt_pointer,
                    sizeof(gdt_pointer)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x0110u, idt_pointer,
                    sizeof(idt_pointer)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x0300u, gdt, sizeof(gdt)) !=
                    LIB_STATUS_OK || core_machine_memory_write(machine, 0x0400u,
                    idt, sizeof(idt)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x0500u, tss, sizeof(tss)) !=
                    LIB_STATUS_OK || core_machine_memory_write(machine, 0u,
                    bootstrap, sizeof(bootstrap)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x3000u, user_code,
                    sizeof(user_code)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x2100u, &handler,
                    sizeof(handler)) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x0600u, source,
                    sizeof(source)) != LIB_STATUS_OK ||
                test_core_machine_fixture_run_after_delivery(machine,
                    (core_machine_run_budget){64u,0u}, &result) != LIB_STATUS_OK ||
                core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                    LIB_STATUS_OK || core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed)
            failed = result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                diagnostic.first_fault.valid ||
                !diagnostic.last_delivered_exception.valid ||
                !(diagnostic.last_delivered_exception.exception_mask &
                    VCPUINS_EXCEPT_GP) || after.eip != 0x0101u ||
                after.cs.selector != 0x0008u || after.gdtr.base != 0x0300u ||
                after.gdtr.limit != 0x002fu || after.idtr.base != 0x0400u ||
                after.idtr.limit != 0x0107u;
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    static const lib_u8 sgdt[] = {0x0fu,0x01u,0x06u,0x00u,0x30u};
    static const lib_u8 sidt[] = {0x0fu,0x01u,0x0eu,0x00u,0x30u};
    static const lib_u8 lgdt[] = {0x0fu,0x01u,0x16u,0x00u,0x02u};
    static const lib_u8 lidt[] = {0x0fu,0x01u,0x1eu,0x00u,0x02u};

    lib_i32 a = table_register_board_case(sgdt, sizeof(sgdt), LIB_FALSE);
    lib_i32 b = table_register_board_case(sidt, sizeof(sidt), LIB_FALSE);
    lib_i32 c = table_register_board_case(lgdt, sizeof(lgdt), LIB_FALSE);
    lib_i32 d = table_register_board_case(lidt, sizeof(lidt), LIB_TRUE);
    lib_i32 e = table_register_board_ltr_memory();
    lib_i32 f = table_register_board_segment_sources();
    lib_i32 g = table_register_board_source_limit();
    lib_i32 h = table_register_board_dos_sgdt_discriminator();
    lib_i32 i = table_register_board_lgdt_consumer();
    lib_i32 j = table_register_board_cpl_reject();

    if (!a || !b || !c || !d || !e || !f || !g || !h || !i || !j) {
        fprintf(stderr, "M5:T539:S42:table-register board failed sgdt=%d sidt=%d lgdt=%d lidt=%d ltr=%d segments=%d limit=%d dos=%d consumer=%d cpl=%d\n",
            a, b, c, d, e, f, g, h, i, j);
        return 1;
    }
    puts("M5:T539:S42:TABLE-REGISTER-BOARD:OK");
    return 0;
}
