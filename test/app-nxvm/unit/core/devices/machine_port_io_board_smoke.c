#include "support/core_machine_board_fixture.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "support/pic_fixture.h"
#include "x86/core/device_support_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include <stdio.h>

typedef struct port_io_board_probe {
    lib_u32 input;
    lib_u32 last_write;
    lib_u16 last_port;
    lib_u32 reads;
    lib_u32 writes;
    lib_bool fail;
    lib_u64 read_tick;
} port_io_board_probe;

static lib_status port_io_board_read(void *opaque, lib_u16 port, lib_u64 tick,
    lib_u32 *value)
{
    port_io_board_probe *probe = (port_io_board_probe *)opaque;

    if (probe->fail) return LIB_STATUS_INVALID_ARGUMENT;
    ++probe->reads;
    probe->read_tick = tick;
    probe->last_port = port;
    *value = probe->input;
    return LIB_STATUS_OK;
}

static lib_status port_io_board_write(void *opaque, lib_u16 port,
    lib_u32 value)
{
    port_io_board_probe *probe = (port_io_board_probe *)opaque;

    if (probe->fail) return LIB_STATUS_INVALID_ARGUMENT;
    ++probe->writes;
    probe->last_port = port;
    probe->last_write = value;
    return LIB_STATUS_OK;
}

static lib_i32 port_io_board_prepare(core_machine **out_machine,
    port_io_board_probe *probe, core_machine_cpu_profile profile,
    core_machine_board_state **out_board)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_port_provider provider = {
        port_io_board_read, port_io_board_write
    };
    const core_machine_debug_register_patch patch = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {
            [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
            [CORE_MACHINE_DEBUG_EAX] = 0xa1a1b2b2u,
            [CORE_MACHINE_DEBUG_EDX] = 0x00e0u,
            [CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_IF
        }
    };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = core_machine_create(&config, &machine, out_board) != LIB_STATUS_OK;

    if (!failed)
        failed = core_machine_install_port_provider(machine, 0x005au, 0x005au,
            &provider, probe) != LIB_STATUS_OK ||
            core_machine_install_port_provider(machine, 0x00e0u, 0x00e0u,
                &provider, probe) != LIB_STATUS_OK ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK;
    if (failed) {
        core_machine_destroy(machine);
        if (out_board != LIB_NULL) *out_board = LIB_NULL;
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 port_io_board_provider_failure(
    core_machine_cpu_profile profile, lib_bool input)
{
    static const lib_u8 in_code[] = {0xe4u,0x5au};
    static const lib_u8 out_code[] = {0xe7u,0x5au};
    const lib_u8 *code = input ? in_code : out_code;
    port_io_board_probe probe = {0};
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_i32 failed = !port_io_board_prepare(&machine, &probe, profile, LIB_NULL);

    probe.fail = LIB_TRUE;
    if (!failed)
        failed = core_machine_memory_write(machine, 0u, code, 2u) !=
            LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u,0u},
                &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
    if (!failed)
        failed = !diagnostic.first_fault.valid ||
            !(diagnostic.first_fault.exception_mask & VCPUINS_EXCEPT_CE) ||
            after.eip != 0u || after.eax != 0xa1a1b2b2u ||
            probe.reads != 0u || probe.writes != 0u;
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 port_io_board_irq(lib_bool input)
{
    static const lib_u8 in_code[] = {0xe4u,0x5au,0x90u};
    static const lib_u8 out_code[] = {0xeeu,0x90u};
    static const lib_u8 halt = 0xf4u;
    const lib_u8 *code = input ? in_code : out_code;
    lib_u8 bytes = input ? sizeof(in_code) : sizeof(out_code);
    port_io_board_probe probe = {0};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_pic_irq_source irq = {0};
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_u16 offset = 0x100u, segment = 0u, frame_ip = 0xffffu;
    lib_i32 failed = !port_io_board_prepare(&machine, &probe,
        CORE_MACHINE_CPU_PROFILE_80386, &board);

    probe.input = 0x11223344u;
    if (!failed)
        failed = core_machine_memory_write(machine, 0u, code, bytes) !=
            LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x80u, &offset,
                sizeof(offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x100u, &halt,
                sizeof(halt)) != LIB_STATUS_OK;
    if (!failed) {
        test_pic_program_vector(&board->shared_pic_master, 0x20u);
        core_machine_pic_irq_source_bind(&irq, &board->shared_pic_master,
            &board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed = core_machine_run(machine, (core_machine_run_budget){2u,0u},
            &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine,
                after.ss.base + (lib_u16)after.esp, &frame_ip,
                sizeof(frame_ip)) != LIB_STATUS_OK;
    }
    if (!failed)
        failed = after.eip != 0x101u ||
            frame_ip != (input ? 2u : 1u) ||
            after.eax != (input ? 0xa1a1b244u : 0xa1a1b2b2u) ||
            probe.reads != (input ? 1u : 0u) ||
            probe.writes != (input ? 0u : 1u) ||
            probe.last_port != (input ? 0x005au : 0x00e0u) ||
            (!input && probe.last_write != 0xb2u) ||
            !(test_pic_read(&board->shared_pic_master, 0x0bu) &
                VPIC_ISR_IRQ(0u)) ||
            (test_pic_read(&board->shared_pic_master, 0x0au) &
                VPIC_IRR_IRQ(0u));
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 port_io_board_time(core_machine_cpu_profile profile)
{
    const lib_u8 code[] = {0x90u, 0xe4u, 0x5au};
    port_io_board_probe probe = {0};
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_time_observation time = {0};
    lib_i32 failed = !port_io_board_prepare(&machine, &probe, profile, LIB_NULL);

    if (!failed)
        failed = core_machine_memory_write(machine, 0u, code, sizeof(code)) !=
            LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u, 0u},
                &result) != LIB_STATUS_OK ||
            core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
            time.elapsed_ticks == 0u ||
            core_machine_run(machine, (core_machine_run_budget){1u, 0u},
                &result) != LIB_STATUS_OK ||
            probe.reads != 1u || probe.read_tick != time.elapsed_ticks;
    core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!port_io_board_provider_failure(CORE_MACHINE_CPU_PROFILE_8086,
            LIB_TRUE) ||
        !port_io_board_provider_failure(CORE_MACHINE_CPU_PROFILE_8086,
            LIB_FALSE) ||
        !port_io_board_provider_failure(CORE_MACHINE_CPU_PROFILE_80386,
            LIB_TRUE) ||
        !port_io_board_provider_failure(CORE_MACHINE_CPU_PROFILE_80386,
            LIB_FALSE) ||
        !port_io_board_irq(LIB_TRUE) || !port_io_board_irq(LIB_FALSE) ||
        !port_io_board_time(CORE_MACHINE_CPU_PROFILE_8086) ||
        !port_io_board_time(CORE_MACHINE_CPU_PROFILE_80386)) {
        printf("Scalar port I/O board delivery failed\n");
        return 1;
    }
    printf("M5:T539:S39:PORT-IO-BOARD:OK\n");
    return 0;
}
