#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/media_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct core_machine_port_probe_state {
    uint32_t reads;
    uint32_t writes;
    uint32_t last_write;
} core_machine_port_probe_state;

static type_status core_machine_port_probe_read(C_VOID *owner, uint16_t port,
    uint32_t *out_value)
{
    core_machine_port_probe_state *state =
        (core_machine_port_probe_state *)owner;

    if (state == STD_NULL || out_value == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ++state->reads;
    *out_value = port == 0x00e0u ? 0xa5u : 0x5au;
    return TYPE_STATUS_OK;
}

static type_status core_machine_port_probe_write(C_VOID *owner, uint16_t port,
    uint32_t value)
{
    core_machine_port_probe_state *state =
        (core_machine_port_probe_state *)owner;

    if (state == STD_NULL || (port != 0x00e0u && port != 0x03f2u)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ++state->writes;
    state->last_write = value;
    return TYPE_STATUS_OK;
}

static C_INT core_machine_port_probe_prepare(core_machine **out_machine,
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
    core_machine *machine = STD_NULL;
    C_INT failed = out_machine == STD_NULL || port_state == STD_NULL;

    failed |= !failed && core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_install_port_provider(machine, 0x00e0u,
        0x00e0u, &provider, port_state) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_install_port_provider(machine, 0x00e0u,
        0x00e0u, &provider, port_state) != TYPE_STATUS_INVALID_STATE;
    failed |= !failed && core_machine_install_port_provider(machine, 0x0020u,
        0x0020u, &provider, port_state) != TYPE_STATUS_INVALID_STATE;
    failed |= !failed && core_machine_install_port_provider(machine, 0x0040u,
        0x0043u, &provider, port_state) != TYPE_STATUS_INVALID_STATE;
    failed |= !failed && core_machine_install_port_provider(machine, 0x0060u,
        0x0064u, &provider, port_state) != TYPE_STATUS_INVALID_STATE;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    if (failed) {
        core_machine_destroy(machine);
        return 1;
    }
    *out_machine = machine;
    return 0;
}

static C_INT core_machine_port_probe_fdc_read_is_independent(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_port_provider read_provider = {
        core_machine_port_probe_read, STD_NULL
    };
    const core_machine_dma_wiring dma_wiring = {.fdc_channel = 2u};
    core_machine_media_registry media = {0};
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_topology topology = {
        .media_registry = &media,
        .drives = {{1u, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}},
        .config = {
            .dor_port = 0x03f2u, .status_port = 0x03f4u,
            .data_port = 0x03f5u, .direction_port = 0x03f7u,
            .control_port = 0x03f7u, .irq = 6u, .dma_channel = 2u
        }
    };
    core_machine_port_probe_state state = {0u, 0u, 0u};
    core_machine *machine = STD_NULL;
    uint32_t value = 0u;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_configure_dma(machine, &dma_wiring, &dma_request) !=
            TYPE_STATUS_OK ||
        core_machine_install_port_provider(machine, 0x03f2u, 0x03f2u,
            &read_provider, &state) != TYPE_STATUS_OK;

    topology.dma_request = dma_request;
    failed |= !failed && core_machine_configure_fdc(machine, &topology) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && (core_machine_bus_read(machine, 0x03f2u, &value) !=
            TYPE_STATUS_OK || value != 0x5au || state.reads != 1u ||
        core_machine_bus_write(machine, 0x03f2u, 0x1cu) != TYPE_STATUS_OK ||
        machine->fdc.data.dor != 0x1cu || state.writes != 0u);
    core_machine_destroy(machine);
    return failed;
}

static C_INT core_machine_port_probe_fdc_write_conflict_is_retained(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_port_provider write_provider = {
        STD_NULL, core_machine_port_probe_write
    };
    const core_machine_dma_wiring dma_wiring = {.fdc_channel = 2u};
    core_machine_media_registry media = {0};
    core_machine_dma_request_binding dma_request = {0};
    core_machine_fdc_topology topology = {
        .media_registry = &media,
        .drives = {{1u, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}},
        .config = {
            .dor_port = 0x03f2u, .status_port = 0x03f4u,
            .data_port = 0x03f5u, .direction_port = 0x03f7u,
            .control_port = 0x03f7u, .irq = 6u, .dma_channel = 2u
        }
    };
    core_machine_port_probe_state state = {0u, 0u, 0u};
    core_machine *machine = STD_NULL;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_configure_dma(machine, &dma_wiring, &dma_request) !=
            TYPE_STATUS_OK ||
        core_machine_install_port_provider(machine, 0x03f2u, 0x03f2u,
            &write_provider, &state) != TYPE_STATUS_OK;

    topology.dma_request = dma_request;
    failed |= !failed && core_machine_configure_fdc(machine, &topology) !=
        TYPE_STATUS_INVALID_STATE;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && (core_machine_bus_write(machine, 0x03f2u, 0x4du) !=
            TYPE_STATUS_OK || state.writes != 1u || state.last_write != 0x4du);
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    static const uint8_t program[] = {
        0xb0u, 0x5au, 0xe6u, 0xe0u, 0xe4u, 0xe0u, 0xf4u
    };
    core_machine_port_probe_state port_state = {0u, 0u, 0u};
    core_machine_run_budget budget = {16u, 0u};
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    uint32_t value = 0u;
    C_INT failed = core_machine_port_probe_prepare(&machine, &port_state);

    if (!failed) {
        failed |= core_machine_bus_write(machine, 0x00e0u, 0x33u) !=
                TYPE_STATUS_OK ||
            core_machine_bus_read(machine, 0x00e0u, &value) != TYPE_STATUS_OK ||
            value != 0xa5u ||
            !test_core_machine_fixture_prepare_real_mode_execution(machine, 0u) ||
            core_machine_memory_write(machine, 0u, program, sizeof(program)) !=
                TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            port_state.reads != 2u || port_state.writes != 2u ||
            port_state.last_write != 0x5au;
    }
    core_machine_destroy(machine);
    failed |= core_machine_port_probe_fdc_read_is_independent();
    failed |= core_machine_port_probe_fdc_write_conflict_is_retained();
    if (failed) {
        STD_FPRINTF(STD_STDERR,
            "M5:T300:S1:PORT-OWNERSHIP:FAIL reads=%u writes=%u\n",
            port_state.reads, port_state.writes);
        return 1;
    }
    puts("M5:T300:S1:PORT-OWNERSHIP:OK");
    return 0;
}
