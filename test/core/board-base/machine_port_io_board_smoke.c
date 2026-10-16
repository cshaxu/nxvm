#include "lib/types/file.h"
#include "core_machine_board_fixture.h"
#include "core/board-base/machine_board_state.h"
#include "pic_fixture.h"
#include "core/board-base/machine_board_interface.h"
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

static lib_i32 port_io_board_prepare(x86_core_machine **out_machine,
    port_io_board_probe *probe, x86_core_cpu_profile profile,
    core_machine_board_state **out_board)
{
    const core_machine_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const x86_core_machine_port_provider provider = {
        port_io_board_read, port_io_board_write
    };
    const x86_core_cpu_debug_register_patch patch = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
        .values = {
            [X86_CORE_CPU_DEBUG_ESP] = 0x8000u,
            [X86_CORE_CPU_DEBUG_EAX] = 0xa1a1b2b2u,
            [X86_CORE_CPU_DEBUG_EDX] = 0x00e0u,
            [X86_CORE_CPU_DEBUG_EFLAGS] = X86_CORE_CPU_DEBUG_EFLAGS_IF
        }
    };
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = core_machine_create(&config, &machine, out_board) != LIB_STATUS_OK;

    if (!failed)
        failed = x86_core_machine_install_port_provider(machine, 0x005au, 0x005au,
            &provider, probe) != LIB_STATUS_OK ||
            x86_core_machine_install_port_provider(machine, 0x00e0u, 0x00e0u,
                &provider, probe) != LIB_STATUS_OK ||
            x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK;
    if (failed) {
        x86_core_machine_destroy(machine);
        if (out_board != LIB_NULL) *out_board = LIB_NULL;
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 port_io_board_provider_failure(
    x86_core_cpu_profile profile, lib_bool input)
{
    static const lib_u8 in_code[] = {0xe4u,0x5au};
    static const lib_u8 out_code[] = {0xe7u,0x5au};
    const lib_u8 *code = input ? in_code : out_code;
    port_io_board_probe probe = {0};
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_diagnostic diagnostic = {0};
    x86_core_cpu_debug_cpu_snapshot after = {0};
    lib_i32 failed = !port_io_board_prepare(&machine, &probe, profile, LIB_NULL);

    probe.fail = LIB_TRUE;
    if (!failed)
        failed = x86_core_machine_memory_write(machine, 0u, code, 2u) !=
            LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u},
                &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != X86_CORE_MACHINE_STOP_FAULT ||
            x86_core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                LIB_STATUS_OK ||
            x86_core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
    if (!failed)
        failed = !diagnostic.first_fault.valid ||
            !(diagnostic.first_fault.exception_mask & VCPUINS_EXCEPT_CE) ||
            after.eip != 0u || after.eax != 0xa1a1b2b2u ||
            probe.reads != 0u || probe.writes != 0u;
    x86_core_machine_destroy(machine);
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
    x86_core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_pic_irq_source *irq = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot after = {0};
    lib_u16 offset = 0x100u, segment = 0u, frame_ip = 0xffffu;
    lib_i32 failed = !port_io_board_prepare(&machine, &probe,
        X86_CORE_CPU_PROFILE_80386, &board);

    probe.input = 0x11223344u;
    if (!failed)
        failed = x86_core_machine_memory_write(machine, 0u, code, bytes) !=
            LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x80u, &offset,
                sizeof(offset)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x100u, &halt,
                sizeof(halt)) != LIB_STATUS_OK;
    if (!failed) {
        test_pic_program_vector(board->shared_pic_master, 0x20u);
        test_pic_bind_source(&irq, board->shared_pic_master,
            board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(irq);
        core_machine_pic_irq_source_deassert(irq);
        failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){2u,0u},
            &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            x86_core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            x86_core_machine_debug_read_memory(machine,
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
            !(test_pic_read(board->shared_pic_master, 0x0bu) &
                VPIC_ISR_IRQ(0u)) ||
            (test_pic_read(board->shared_pic_master, 0x0au) &
                VPIC_IRR_IRQ(0u));
    x86_core_machine_destroy(machine);
    return !failed;
}

static lib_i32 port_io_board_time(x86_core_cpu_profile profile)
{
    const lib_u8 code[] = {0x90u, 0xe4u, 0x5au};
    port_io_board_probe probe = {0};
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_machine_time_observation time = {0};
    lib_i32 failed = !port_io_board_prepare(&machine, &probe, profile, LIB_NULL);

    if (!failed)
        failed = x86_core_machine_memory_write(machine, 0u, code, sizeof(code)) !=
            LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
                &result) != LIB_STATUS_OK ||
            x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
            time.elapsed_ticks == 0u ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
                &result) != LIB_STATUS_OK ||
            probe.reads != 1u || probe.read_tick != time.elapsed_ticks;
    x86_core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!port_io_board_provider_failure(X86_CORE_CPU_PROFILE_8086,
            LIB_TRUE) ||
        !port_io_board_provider_failure(X86_CORE_CPU_PROFILE_8086,
            LIB_FALSE) ||
        !port_io_board_provider_failure(X86_CORE_CPU_PROFILE_80386,
            LIB_TRUE) ||
        !port_io_board_provider_failure(X86_CORE_CPU_PROFILE_80386,
            LIB_FALSE) ||
        !port_io_board_irq(LIB_TRUE) || !port_io_board_irq(LIB_FALSE) ||
        !port_io_board_time(X86_CORE_CPU_PROFILE_8086) ||
        !port_io_board_time(X86_CORE_CPU_PROFILE_80386)) {
        lib_c_printf("Scalar port I/O board delivery failed\n");
        return 1;
    }
    lib_c_printf("PORT-IO-BOARD:OK\n");
    return 0;
}
