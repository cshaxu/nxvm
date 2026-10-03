#include "support/core_machine_board_fixture.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "support/pic_fixture.h"
#include "app-nxvm/devices/device_support.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include <stdio.h>

typedef struct port_strings_board_probe {
    lib_u32 input;
    lib_u32 reads;
    lib_u32 writes;
    lib_u32 last_write;
} port_strings_board_probe;

static lib_status port_strings_board_read(void *opaque, lib_u16 port, lib_u64 tick,
    lib_u32 *value)
{
    (void)tick;
    port_strings_board_probe *probe = (port_strings_board_probe *)opaque;

    if (port != 0x00e0u) return LIB_STATUS_INVALID_ARGUMENT;
    ++probe->reads;
    *value = probe->input;
    return LIB_STATUS_OK;
}

static lib_status port_strings_board_write(void *opaque, lib_u16 port,
    lib_u32 value)
{
    port_strings_board_probe *probe = (port_strings_board_probe *)opaque;

    if (port != 0x00e0u) return LIB_STATUS_INVALID_ARGUMENT;
    ++probe->writes;
    probe->last_write = value;
    return LIB_STATUS_OK;
}

static lib_i32 port_strings_board_prepare(core_machine **out_machine,
    port_strings_board_probe *probe)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_port_provider provider = {
        port_strings_board_read, port_strings_board_write
    };
    const core_machine_debug_register_patch patch = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {
            [CORE_MACHINE_DEBUG_DS] = 0x2000u,
            [CORE_MACHINE_DEBUG_ES] = 0x3000u,
            [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
            [CORE_MACHINE_DEBUG_EDX] = 0x778800e0u,
            [CORE_MACHINE_DEBUG_ESI] = 0x10u,
            [CORE_MACHINE_DEBUG_EDI] = 0x20u,
            [CORE_MACHINE_DEBUG_ECX] = 0x11220003u,
            [CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_IF
        }
    };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = core_machine_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed)
        failed = core_machine_install_port_provider(machine, 0x00e0u,
            0x00e0u, &provider, probe) != LIB_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK;
    if (failed) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 port_strings_board_irq(lib_bool input, lib_bool repeat)
{
    static const lib_u8 ins[] = {0x6cu,0x90u};
    static const lib_u8 outs[] = {0x6eu,0x90u};
    static const lib_u8 rep_ins[] = {0xf3u,0x6cu,0x90u};
    static const lib_u8 rep_outs[] = {0xf3u,0x6eu,0x90u};
    static const lib_u8 hlt = 0xf4u;
    const lib_u8 *code = repeat ? (input ? rep_ins : rep_outs) :
        (input ? ins : outs);
    lib_u8 bytes = repeat ? 3u : 2u;
    const lib_u8 source[] = {0x5au,0x6bu,0x7cu};
    port_strings_board_probe probe = {0};
    core_machine *machine = LIB_NULL;
    core_machine_pic_irq_source irq = {0};
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_u16 offset = 0x100u, segment = 0u, frame_ip = 0xffffu;
    lib_u8 observed = 0u;
    lib_i32 failed = !port_strings_board_prepare(&machine, &probe);

    probe.input = source[0];
    if (!failed)
        failed = core_machine_memory_write(machine, 0x20010u, source,
            sizeof(source)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x80u, &offset,
                sizeof(offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x100u, &hlt,
                sizeof(hlt)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0u, code, bytes) != LIB_STATUS_OK;
    if (!failed) {
        test_pic_program_vector(&machine->board->shared_pic_master, 0x20u);
        core_machine_pic_irq_source_bind(&irq, &machine->board->shared_pic_master,
            &machine->board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed = core_machine_run(machine,
            (core_machine_run_budget){repeat ? 4u : 2u,0u}, &result) !=
            LIB_STATUS_OK || result.reason !=
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine,
                after.ss.base + (lib_u16)after.esp, &frame_ip,
                sizeof(frame_ip)) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine,
                input ? 0x30020u : 0x20010u, &observed,
                sizeof(observed)) != LIB_STATUS_OK;
    }
    if (!failed)
        failed = after.eip != 0x101u || frame_ip != (repeat ? 0u : 1u) ||
            after.esi != (input ? 0x10u : 0x11u) ||
            after.edi != (input ? 0x21u : 0x20u) ||
            after.ecx != (repeat ? 0x11220002u : 0x11220003u) ||
            after.eflags != 0u ||
            probe.reads != (input ? 1u : 0u) ||
            probe.writes != (input ? 0u : 1u) ||
            (!input && probe.last_write != source[0]) ||
            observed != source[0] ||
            !(test_pic_read(&machine->board->shared_pic_master, 0x0bu) &
                VPIC_ISR_IRQ(0u)) ||
            (test_pic_read(&machine->board->shared_pic_master, 0x0au) &
                VPIC_IRR_IRQ(0u));
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 port_strings_board_protected(lib_bool input, lib_bool repeat)
{
    static const lib_u8 pointer[] = {0x27u,0,0,0x03u,0,0};
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0,1u, 0xb8u,1u,0, 0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0,0x8eu,0xd8u, 0xb8u,0x18u,0,0x8eu,0xc0u,
        0xb8u,0x20u,0,0x8eu,0xd0u, 0xbcu,0,0x80u,
        0xeau,0,0,8u,0
    };
    static const lib_u8 ins[] = {0x6cu}, outs[] = {0x6eu};
    static const lib_u8 rep_ins[] = {0xf3u,0x6cu};
    static const lib_u8 rep_outs[] = {0xf3u,0x6eu};
    const lib_u8 *code = repeat ? (input ? rep_ins : rep_outs) :
        (input ? ins : outs);
    lib_u8 bytes = repeat ? 2u : 1u;
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x92u,0,0
    };
    const lib_u8 source[] = {0x5au,0x6bu};
    const lib_u8 destination[] = {0xa5u,0xb6u};
    port_strings_board_probe probe = {0};
    core_machine *machine = LIB_NULL;
    core_machine_debug_register_patch entry = {0};
    core_machine_debug_register_patch patch = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_run_result result = {0};
    lib_i32 failed = !port_strings_board_prepare(&machine, &probe);

    gdt[input ? 24u : 16u] = repeat ? 0x10u : 0x0fu;
    gdt[input ? 25u : 17u] = 0u;
    if (!failed) {
        entry.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP);
        failed = core_machine_debug_patch_registers(machine, &entry) !=
            LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x100u, pointer,
                sizeof(pointer)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x300u, gdt,
                sizeof(gdt)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0u, bootstrap,
                sizeof(bootstrap)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x2000u, code,
                bytes) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){11u,0u},
                &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 11u;
    }
    if (!failed) {
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
        patch.values[CORE_MACHINE_DEBUG_ECX] = 0x11220003u;
        patch.values[CORE_MACHINE_DEBUG_EDX] = 0x00e0u;
        patch.values[CORE_MACHINE_DEBUG_ESI] = 0x10u;
        patch.values[CORE_MACHINE_DEBUG_EDI] = 0x10u;
        patch.values[CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_IF;
        probe.input = source[0];
        failed = core_machine_debug_patch_registers(machine, &patch) !=
            LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x3010u, source,
                sizeof(source)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x4010u, destination,
                sizeof(destination)) != LIB_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine,
            (core_machine_run_budget){repeat ? 2u : 1u,0u}, &result) !=
            LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
    }
    if (!failed)
        failed = !diagnostic.first_fault.valid ||
            !(diagnostic.first_fault.exception_mask & VCPUINS_EXCEPT_DF) ||
            after.eip != 0u ||
            after.ecx != (repeat ? 0x11220002u : 0x11220003u) ||
            after.esi != (repeat && !input ? 0x11u : 0x10u) ||
            after.edi != (repeat && input ? 0x11u : 0x10u) ||
            after.eflags != VCPU_EFLAGS_IF ||
            probe.reads != (repeat && input ? 1u : 0u) ||
            probe.writes != (repeat && !input ? 1u : 0u) ||
            (repeat && !input && probe.last_write != source[0]);
    core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
#define CHECK(call) do { if (!(call)) { printf("Failed: %s\n", #call); return 1; } } while (0)
    CHECK(port_strings_board_irq(LIB_TRUE, LIB_FALSE));
    CHECK(port_strings_board_irq(LIB_FALSE, LIB_FALSE));
    CHECK(port_strings_board_irq(LIB_TRUE, LIB_TRUE));
    CHECK(port_strings_board_irq(LIB_FALSE, LIB_TRUE));
    CHECK(port_strings_board_protected(LIB_TRUE, LIB_FALSE));
    CHECK(port_strings_board_protected(LIB_FALSE, LIB_FALSE));
    CHECK(port_strings_board_protected(LIB_TRUE, LIB_TRUE));
    CHECK(port_strings_board_protected(LIB_FALSE, LIB_TRUE));
#undef CHECK
    printf("M5:T539:S39:PORT-STRINGS-BOARD:OK\n");
    return 0;
}
