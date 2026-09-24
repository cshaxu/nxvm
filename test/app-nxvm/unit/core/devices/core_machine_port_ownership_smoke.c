#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/media_interface.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct core_machine_port_probe_state {
    lib_u32 reads;
    lib_u32 writes;
    lib_u32 last_write;
    lib_status read_status;
    lib_status write_status;
} core_machine_port_probe_state;

static lib_status core_machine_port_probe_read(void *owner, lib_u16 port,
    lib_u32 *out_value)
{
    core_machine_port_probe_state *state =
        (core_machine_port_probe_state *)owner;

    if (state == LIB_NULL || out_value == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (state->read_status != LIB_STATUS_OK) return state->read_status;
    ++state->reads;
    *out_value = port == 0x00e0u ? 0xa5u : 0x5au;
    return LIB_STATUS_OK;
}

static lib_status core_machine_port_probe_write(void *owner, lib_u16 port,
    lib_u32 value)
{
    core_machine_port_probe_state *state =
        (core_machine_port_probe_state *)owner;

    if (state == LIB_NULL || (port != 0x00e0u && port != 0x03f2u)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (state->write_status != LIB_STATUS_OK) return state->write_status;
    ++state->writes;
    state->last_write = value;
    return LIB_STATUS_OK;
}

static lib_i32 core_machine_port_probe_prepare(core_machine **out_machine,
    core_machine_port_probe_state *port_state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_port_provider provider = {
        core_machine_port_probe_read, core_machine_port_probe_write
    };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = out_machine == LIB_NULL || port_state == LIB_NULL;

    failed |= !failed && core_machine_create(&config, &machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_install_port_provider(machine, 0x00e0u,
        0x00e0u, &provider, port_state) != LIB_STATUS_OK;
    failed |= !failed && core_machine_install_port_provider(machine, 0x00e0u,
        0x00e0u, &provider, port_state) != LIB_STATUS_INVALID_STATE;
    failed |= !failed && core_machine_install_port_provider(machine, 0x0020u,
        0x0020u, &provider, port_state) != LIB_STATUS_INVALID_STATE;
    failed |= !failed && core_machine_install_port_provider(machine, 0x0040u,
        0x0043u, &provider, port_state) != LIB_STATUS_INVALID_STATE;
    failed |= !failed && core_machine_install_port_provider(machine, 0x0060u,
        0x0064u, &provider, port_state) != LIB_STATUS_INVALID_STATE;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    if (failed) {
        core_machine_destroy(machine);
        return 1;
    }
    *out_machine = machine;
    return 0;
}

static lib_i32 core_machine_port_probe_fdc_read_is_independent(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_port_provider read_provider = {
        core_machine_port_probe_read, LIB_NULL
    };
    const core_machine_dma_wiring dma_wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_media_registry *media = LIB_NULL;
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_topology topology = {
        .media_registry = media,
        .drives = {{1u, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}},
        .config = {
            .dor_port = 0x03f2u, .status_port = 0x03f4u,
            .data_port = 0x03f5u, .direction_port = 0x03f7u,
            .control_port = 0x03f7u, .irq = 6u, .dma_channel = 2u
        }
    };
    core_machine_port_probe_state state = {0u, 0u, 0u, LIB_STATUS_OK,
        LIB_STATUS_OK};
    core_machine *machine = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = core_machine_media_registry_create(&media) != LIB_STATUS_OK ||
        core_machine_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_configure_dma(machine, &dma_wiring, &dma_request) !=
            LIB_STATUS_OK ||
        core_machine_install_port_provider(machine, 0x03f2u, 0x03f2u,
            &read_provider, &state) != LIB_STATUS_OK;

    topology.media_registry = media;
    topology.dma_request = dma_request;
    failed |= !failed && core_machine_configure_fdc(machine, &topology) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && (core_machine_bus_read(machine, 0x03f2u, &value) !=
            LIB_STATUS_OK || value != 0x5au || state.reads != 1u ||
        core_machine_bus_write(machine, 0x03f2u, 0x1cu) != LIB_STATUS_OK ||
        machine->fdc.data.dor != 0x1cu || state.writes != 0u);
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    return failed;
}

static lib_i32 core_machine_port_probe_fdc_write_conflict_is_retained(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_port_provider write_provider = {
        LIB_NULL, core_machine_port_probe_write
    };
    const core_machine_dma_wiring dma_wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_media_registry *media = LIB_NULL;
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_topology topology = {
        .media_registry = media,
        .drives = {{1u, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}},
        .config = {
            .dor_port = 0x03f2u, .status_port = 0x03f4u,
            .data_port = 0x03f5u, .direction_port = 0x03f7u,
            .control_port = 0x03f7u, .irq = 6u, .dma_channel = 2u
        }
    };
    core_machine_port_probe_state state = {0u, 0u, 0u, LIB_STATUS_OK,
        LIB_STATUS_OK};
    core_machine *machine = LIB_NULL;
    lib_i32 failed = core_machine_media_registry_create(&media) != LIB_STATUS_OK ||
        core_machine_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_configure_dma(machine, &dma_wiring, &dma_request) !=
            LIB_STATUS_OK ||
        core_machine_install_port_provider(machine, 0x03f2u, 0x03f2u,
            &write_provider, &state) != LIB_STATUS_OK;

    topology.media_registry = media;
    topology.dma_request = dma_request;
    failed |= !failed && core_machine_configure_fdc(machine, &topology) !=
        LIB_STATUS_INVALID_STATE;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && (core_machine_bus_write(machine, 0x03f2u, 0x4du) !=
            LIB_STATUS_OK || state.writes != 1u || state.last_write != 0x4du);
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    return failed;
}

lib_i32 main(void)
{
    static const lib_u8 program[] = {
        0xb0u, 0x5au, 0xe6u, 0xe0u, 0xe4u, 0xe0u, 0xf4u
    };
    static const lib_u8 failing_program[] = {0xb0u, 0x6cu, 0xe6u, 0xe0u};
    core_machine_port_probe_state port_state = {0u, 0u, 0u, LIB_STATUS_OK,
        LIB_STATUS_OK};
    core_machine_run_budget budget = {16u, 0u};
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = core_machine_port_probe_prepare(&machine, &port_state);

    if (!failed) {
        failed |= core_machine_bus_write(machine, 0x00e0u, 0x33u) !=
                LIB_STATUS_OK ||
            core_machine_bus_read(machine, 0x00e0u, &value) != LIB_STATUS_OK ||
            value != 0xa5u ||
            !test_core_machine_fixture_prepare_real_mode_execution(machine, 0u) ||
            core_machine_memory_write(machine, 0u, program, sizeof(program)) !=
                LIB_STATUS_OK ||
            core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            port_state.reads != 2u || port_state.writes != 2u ||
            port_state.last_write != 0x5au;
        port_state.read_status = LIB_STATUS_INTERNAL_ERROR;
        value = 0xdeadbeefu;
        failed |= core_machine_bus_read(machine, 0x00e0u, &value) !=
                LIB_STATUS_INTERNAL_ERROR || value != 0xdeadbeefu;
        port_state.read_status = LIB_STATUS_OK;
        failed |= core_machine_bus_read(machine, 0x00e0u, &value) !=
                LIB_STATUS_OK || value != 0xa5u;
        port_state.write_status = LIB_STATUS_INTERNAL_ERROR;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK ||
            !test_core_machine_fixture_prepare_real_mode_execution(machine, 0u) ||
            core_machine_memory_write(machine, 0u, failing_program,
                sizeof(failing_program)) != LIB_STATUS_OK ||
            core_machine_run(machine, budget, &result) != LIB_STATUS_INTERNAL_ERROR;
        if (!failed) {
            core_machine_cpu_diagnostic diagnostic;

            failed |= core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                    LIB_STATUS_OK || !diagnostic.first_fault.valid ||
                !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_CE) || diagnostic.first_fault.exception_code !=
                    0x00e0u;
        }
    }
    core_machine_destroy(machine);
    failed |= core_machine_port_probe_fdc_read_is_independent();
    failed |= core_machine_port_probe_fdc_write_conflict_is_retained();
    if (failed) {
        fprintf(stderr,
            "M5:T300:S1:PORT-OWNERSHIP:FAIL reads=%u writes=%u\n",
            port_state.reads, port_state.writes);
        return 1;
    }
    puts("M5:T300:S1:PORT-OWNERSHIP:OK");
    return 0;
}
